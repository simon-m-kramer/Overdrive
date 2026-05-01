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
		if (UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent())
		{
			bCarOnLeft = Detection->IsCarOnLeft();
			bCarOnRight = Detection->IsCarOnRight();
		}
	}
}

void UTurboAction_Yield::Update(float DeltaTime)
{
	if (!Vehicle.IsValid() || !RacingSpline.IsValid() || !AIController.IsValid()) return;

	if (UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent())
	{
		bCarOnLeft = Detection->IsCarOnLeft();
		bCarOnRight = Detection->IsCarOnRight();
	}

	// Standard driving with offset target point
	const FVector TargetPoint = GetTargetPoint();
	const float SteeringInput = CalculateSteering(TargetPoint, DeltaTime);
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
	// No adjacent car: hold the line.
	if (!bCarOnLeft && !bCarOnRight) return Super::GetTargetPoint();

	float Sign = 0.0f;
	if (bCarOnLeft && !bCarOnRight)      Sign = 1.0f;   // car on left -> move right
	else if (bCarOnRight && !bCarOnLeft) Sign = -1.0f;  // car on right -> move left
	// Both sides occupied -> sign stays 0 -> helper returns base target

	return GetTargetPointWithLateralOffset(YieldLateralOffset * Sign);
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

