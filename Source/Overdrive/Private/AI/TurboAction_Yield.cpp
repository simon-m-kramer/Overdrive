// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Yield.h"
#include "AI/TurboAIController.h"
#include "AI/TurboAIVehicle.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"

UTurboAction_Yield::UTurboAction_Yield()
{
	ActionName = TEXT("Yield");
	ActionTag = TurboGameplayTags::Action_Yield;

	BlocksTags.AddTag(TurboGameplayTags::Action_Yield);
	BlockedByTags.AddTag(TurboGameplayTags::Action_Overtake);
}

// =============================================================================
// ACTIVATION
// =============================================================================

bool UTurboAction_Yield::CanActivate(const FTurboDecisionContext& Context) const
{
	// Need a car alongside
	if (!Context.bVehicleOnLeft && !Context.bVehicleOnRight) return false;

	// Only yield in curves (I turned off, because MinCurvatureToYield is set 0.0f)
	if (Context.CurrentCurvature < MinCurvatureToYield) return false;

	return true;
}

// =============================================================================
// LIFECYCLE
// =============================================================================

void UTurboAction_Yield::Start(bool bFirstTime)
{
	Super::Start(bFirstTime);

	if (Vehicle.IsValid())
	{
		UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
		if (Detection)
		{
			bCarOnLeft = Detection->IsCarOnLeft();
			bCarOnRight = Detection->IsCarOnRight();
		}
	}
}

void UTurboAction_Yield::Update(float DeltaTime)
{
	if (!Vehicle.IsValid() || !RacingSplineActor.IsValid() || !AIController.IsValid())
		return;

	if (Vehicle.IsValid())
	{
		UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
		if (Detection)
		{
			bCarOnLeft = Detection->IsCarOnLeft();
			bCarOnRight = Detection->IsCarOnRight();
		}
	}

	// Standard driving with offset target point
	FVector TargetPoint = GetTargetPoint();
	float SteeringInput = CalculateSteering(TargetPoint, DeltaTime);
	Vehicle->SetSteeringInput(SteeringInput);

	ApplySpeedControl(DeltaTime);
}

bool UTurboAction_Yield::IsDone()
{
	return IsSideClear();
}

// =============================================================================
// TARGET POINT
// =============================================================================

FVector UTurboAction_Yield::GetTargetPoint()
{
	FVector BaseTarget = Super::GetTargetPoint();

	if (!Vehicle.IsValid() || !AIController.IsValid()) return BaseTarget;

	USplineComponent* Spline = GetSpline();
	if (!Spline) return BaseTarget;

	// If no car alongside anymore, just return base
	if (!bCarOnLeft && !bCarOnRight) return BaseTarget;

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

	float OffsetDirection = 0.0f;

	if (bCarOnLeft && !bCarOnRight)
	{
		OffsetDirection = 1.0f;   // move right
	}
	else if (bCarOnRight && !bCarOnLeft)
	{
		OffsetDirection = -1.0f;  // move left
	}
	// If cars on both sides, don't offset — safest to hold the line

	return BaseTarget + (Right * YieldLateralOffset * OffsetDirection);
}

// =============================================================================
// HELPERS
// =============================================================================

bool UTurboAction_Yield::IsSideClear() const
{
	if (!Vehicle.IsValid())return true;

	UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
	if (!Detection)return true;

	return !Detection->IsCarOnLeft() && !Detection->IsCarOnRight();
}

