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

	// No corner found within scan range
	if (Context.DistanceToNextCorner <= 0.0f || Context.DistanceToNextCorner < MinStraightNeeded) return false;

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
	if (!Vehicle.IsValid() || !RacingSplineActor.IsValid() || !AIController.IsValid())
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
	// Get the base racing line target point
	FVector BaseTarget = Super::GetTargetPoint();

	if (!Vehicle.IsValid()) return BaseTarget;

	// Offset laterally to the chosen side
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

	// Get the spline's right vector at the target distance
	const FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Right = FVector::CrossProduct(Up, Tangent).GetSafeNormal();

	// Apply offset: left = negative right, right = positive right
	const float SideMultiplier = (ChosenSide == EOvertakeSide::Left) ? -1.0f : 1.0f;

	return BaseTarget + (Right * OvertakeLateralOffset * SideMultiplier);
}

float UTurboAction_Overtake::GetTargetSpeedAtDistance(float Distance) const
{
	const float BaseSpeed = Super::GetTargetSpeedAtDistance(Distance);
	return BaseSpeed * OvertakeSpeedBoost;
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
	if (AIController.IsValid() && RacingSplineActor.IsValid())
	{
		const FTurboDecisionContext& Context = AIController->GetDecisionContext();

		if (Context.DistanceToNextCorner > 0.0f)
		{
			const float CurrentDistance = AIController->GetCurrentSplineDistance();
			const float CornerDistance = CurrentDistance + Context.DistanceToNextCorner;
			const float TurnDir = RacingSplineActor->GetTurnSign(CornerDistance);

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
	// Timeout
	if (OvertakeTimer > OvertakeTimeout) return true;

	if (!Vehicle.IsValid()) return true;

	UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
	if (!Detection) return true;

	// Car appeared on our chosen side - no room
	if (ChosenSide == EOvertakeSide::Left && Detection->IsCarOnLeft()) return true;

	if (ChosenSide == EOvertakeSide::Right && Detection->IsCarOnRight()) return true;

	return false;
}