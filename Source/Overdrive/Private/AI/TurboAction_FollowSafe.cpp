// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowSafe.h"
#include "AI/TurboAIController.h"
#include "AI/TurboAIVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboDrivingProfile.h"

UTurboAction_FollowSafe::UTurboAction_FollowSafe()
{
	ActionName = TEXT("FollowSafe");
	ActionTag = TurboGameplayTags::Action_FollowSafe;

	// Block itself
	BlocksTags.AddTag(TurboGameplayTags::Action_FollowSafe);
}

// =============================================================================
// ACTIVATION
// =============================================================================

bool UTurboAction_FollowSafe::CanActivate(const FTurboDecisionContext& Context) const
{
	if (!Context.bVehicleAhead)
		return false;

	if (Context.DistanceToVehicleAhead > FollowReactionDistance)
		return false;

	return true;
}

bool UTurboAction_FollowSafe::IsDone()
{
	if (!AIController.IsValid())
		return true;

	const FTurboDecisionContext& Context = AIController->GetDecisionContext();

	// No car ahead anymore — pop back to follow path
	if (!Context.bVehicleAhead)
		return true;

	// Car moved out of reaction range
	if (Context.DistanceToVehicleAhead > FollowReactionDistance * 1.2f)
		return true;

	return false;
}

// =============================================================================
// SPEED CONTROL
// =============================================================================

void UTurboAction_FollowSafe::ApplySpeedControl(float DeltaTime)
{
	if (!Vehicle.IsValid() || !AIController.IsValid()) return;

	const float CurrentSpeedCms = FMath::Abs(Vehicle->GetForwardSpeed());
	const float CurrentDistance = AIController->GetCurrentSplineDistance();

	const float ProfileSpeed = AIController->GetTargetSpeedAtDistance(CurrentDistance);
	const float FollowLimit = GetFollowSpeedLimit();
	const float TargetSpeedCms = FMath::Min(ProfileSpeed, FollowLimit);

	const float SpeedError = TargetSpeedCms - CurrentSpeedCms;

	float FinalThrottle = 0.0f;
	float FinalBrake = 0.0f;

	const float PIDOutput = SpeedPID.Update(SpeedError, DeltaTime);

	if (PIDOutput > 0.0f)
	{
		FinalThrottle = PIDOutput;
	}
	else
	{
		FinalBrake = -PIDOutput;
	}

	Vehicle->SetThrottleInput(FinalThrottle);
	Vehicle->SetBrakeInput(FinalBrake);
	Vehicle->SetHandbrakeInput(false);
}

float UTurboAction_FollowSafe::GetFollowSpeedLimit() const
{
	if (!AIController.IsValid()) return MAX_FLT;

	const FTurboDecisionContext& Context = AIController->GetDecisionContext();
	if (!Context.bVehicleAhead) return MAX_FLT;

	const float Distance = Context.DistanceToVehicleAhead;
	const float TheirSpeed = Context.SpeedOfVehicleAheadCms;

	if (Distance > FollowReactionDistance) return MAX_FLT;

	if (Distance < FollowEmergencyDistance)
	{
		return TheirSpeed * 0.5f;
	}

	const float T = FMath::Clamp((FollowReactionDistance - Distance) / (FollowReactionDistance - FollowMinDistance), 0.0f, 1.0f);

	const float MatchSpeed = TheirSpeed - FollowSpeedMarginCms;
	const float OurMaxSpeed = AIController->GetTargetSpeedAtDistance(Context.CurrentSplineDistance);

	return FMath::Lerp(OurMaxSpeed, MatchSpeed, T);
}