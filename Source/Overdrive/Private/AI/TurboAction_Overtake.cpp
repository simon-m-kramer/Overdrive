// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Overtake.h"
#include "AI/TurboAIController.h"
#include "AI/TurboAIVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Framework/TurboDrivingProfile.h"

UTurboAction_Overtake::UTurboAction_Overtake()
{
	ActionName = TEXT("Overtake");
	ActionTag = TurboGameplayTags::Action_Overtake;

	BlocksTags.AddTag(TurboGameplayTags::Action_Overtake);
	BlocksTags.AddTag(TurboGameplayTags::Action_FollowSafe);
	BlocksTags.AddTag(TurboGameplayTags::Action_Yield);
}

// =============================================================================
// ACTIVATION
// =============================================================================

bool UTurboAction_Overtake::CanActivate(const FTurboDecisionContext& Context) const
{
	// Is a car ahead?
	if (!Context.bVehicleAhead) return false;

	// Are we close enough?
	if (Context.DistanceToVehicleAhead > OvertakeConsiderationDistance) return false;

	// We must be capable of going faster (profile speed vs their speed)
	const float PotentialAdvantage = Context.TargetSpeedCms - Context.SpeedOfVehicleAheadCms;
	if (PotentialAdvantage < MinSpeedAdvantage) return false;

	// No corner found within scan range = long straight = safe to pass
	if (Context.DistanceToNextCorner > 0.0f && Context.DistanceToNextCorner < MinStraightNeeded) return false;

	return true;
}

// =============================================================================
// LIFECYCLE
// =============================================================================

void UTurboAction_Overtake::Start(bool bFirstTime)
{
	Super::Start(bFirstTime);

	bOvertakeComplete = false;
	bOvertakeAborted = false;
	OvertakeTimer = 0.0f;
	bPulledOut = false;

	// Remember who we're passing
	if (Vehicle.IsValid())
	{
		UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
		if (Detection)
		{
			OvertakeTarget = Detection->GetCarAhead();
		}
	}

	// Choose which side to pass on
	ChosenSide = DetermineBestSide();
}

void UTurboAction_Overtake::Update(float DeltaTime)
{
	if (!Vehicle.IsValid() || !RacingSpline.IsValid() || !AIController.IsValid())
	{
		bOvertakeAborted = true;
		return;
	}

	OvertakeTimer += DeltaTime;

	// Check abort conditions
	if (ShouldAbort())
	{
		bOvertakeAborted = true;
		return;
	}

	// Check completion
	if (IsPassComplete())
	{
		bOvertakeComplete = true;
		return;
	}

	if (!bPulledOut && OvertakeTarget.IsValid())
	{
		const FVector ToTarget = OvertakeTarget->GetActorLocation() - Vehicle->GetActorLocation();
		const float DistanceToTarget = ToTarget.Size();

		const float TheirSpeed = FMath::Abs(OvertakeTarget->GetForwardSpeed());
		const float OurSpeed = FMath::Abs(Vehicle->GetForwardSpeed());
		const float ClosingSpeed = FMath::Max(OurSpeed - TheirSpeed, 0.0f);

		const float DynamicPullOut = MinPullOutDistance + (ClosingSpeed * PullOutTimeFactor);
		bPulledOut = DistanceToTarget < DynamicPullOut;
	}

	// Drive - inherited steering and speed control with our overrides
	FVector TargetPoint = GetTargetPoint();
	float SteeringInput = CalculateSteering(TargetPoint, DeltaTime);
	Vehicle->SetSteeringInput(SteeringInput);

	ApplySpeedControl(DeltaTime);
}

bool UTurboAction_Overtake::IsDone()
{
	return bOvertakeComplete || bOvertakeAborted;
}

// =============================================================================
// OVERRIDES
// =============================================================================

FVector UTurboAction_Overtake::GetTargetPoint()
{
	FVector BaseTarget = Super::GetTargetPoint();

	if (!bPulledOut || !Vehicle.IsValid()) return BaseTarget;

	USplineComponent* Spline = GetSpline();
	if (!Spline || !AIController.IsValid()) return BaseTarget;

	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float LookaheadDist = GetLookaheadDistance();
	float TargetDistance = CurrentDistance + LookaheadDist;

	if (Spline->IsClosedLoop())
	{
		TargetDistance = FMath::Fmod(TargetDistance, Spline->GetSplineLength());
		if (TargetDistance < 0.0f) TargetDistance += Spline->GetSplineLength();
	}

	const FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Right = FVector::CrossProduct(Up, Tangent).GetSafeNormal();

	const float SideMultiplier = (ChosenSide == EOvertakeSide::Left) ? -1.0f : 1.0f;

	return BaseTarget + (Right * OvertakeLateralOffset * SideMultiplier);
}

// =============================================================================
// DECISION HELPERS
// =============================================================================

EOvertakeSide UTurboAction_Overtake::DetermineBestSide() const
{
	if (!Vehicle.IsValid()) return EOvertakeSide::Left;

	UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
	if (!Detection) return EOvertakeSide::Left;

	const bool bLeftClear = Detection->IsOvertakeSafe(EOvertakeSide::Left);
	const bool bRightClear = Detection->IsOvertakeSafe(EOvertakeSide::Right);

	// If only one side is clear, take it
	if (bLeftClear && !bRightClear) return EOvertakeSide::Left;
	if (bRightClear && !bLeftClear) return EOvertakeSide::Right;

	// Both clear - prefer the inside of the next corner for a better exit
	if (AIController.IsValid() && RacingSpline.IsValid())
	{
		const FTurboDecisionContext& Context = AIController->GetDecisionContext();

		if (Context.DistanceToNextCorner > 0.0f)
		{
			const float CurrentDistance = AIController->GetCurrentSplineDistance();
			const float CornerDistance = CurrentDistance + Context.DistanceToNextCorner;
			const float TurnDir = RacingSpline->GetTurnSign(CornerDistance);

			// TurnDir > 0 = right turn -> inside is right
			// TurnDir < 0 = left turn -> inside is left
			if (TurnDir > 0.0f) return EOvertakeSide::Right;
			if (TurnDir < 0.0f) return EOvertakeSide::Left;
		}
	}

	// Default: pass on the left
	return EOvertakeSide::Left;
}

bool UTurboAction_Overtake::IsPassComplete() const
{
	if (!OvertakeTarget.IsValid())
	{
		// Lost track of who we're passing - consider it done
		return true;
	}

	if (!Vehicle.IsValid()) return true;

	// Check if we're sufficiently ahead of the target
	const FVector OurLocation = Vehicle->GetActorLocation();
	const FVector TheirLocation = OvertakeTarget->GetActorLocation();
	const FVector OurForward = Vehicle->GetActorForwardVector();

	const FVector ToThem = TheirLocation - OurLocation;
	const float DotForward = FVector::DotProduct(ToThem, OurForward);

	// Negative dot = they're behind us
	// The more negative, the further behind
	return DotForward < -ClearanceDistanceAhead;
}

bool UTurboAction_Overtake::ShouldAbort() const
{
	if (OvertakeTimer > OvertakeTimeout) return true;

	if (!Vehicle.IsValid()) return true;

	UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
	if (!Detection) return true;

	if (ChosenSide == EOvertakeSide::Left)
	{
		ATurboVehicle* CarOnLeft = Detection->GetCarOnLeft();
		if (CarOnLeft && CarOnLeft != OvertakeTarget.Get()) return true;
	}

	if (ChosenSide == EOvertakeSide::Right)
	{
		ATurboVehicle* CarOnRight = Detection->GetCarOnRight();
		if (CarOnRight && CarOnRight != OvertakeTarget.Get()) return true;
	}

	return false;
}