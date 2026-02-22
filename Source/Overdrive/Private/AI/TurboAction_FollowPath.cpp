// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboActionStack.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"

UTurboAction_FollowPath::UTurboAction_FollowPath()
{
	ActionName = TEXT("FollowPath");
	ActionTag = TurboGameplayTags::Action_FollowPath;

	SteeringPID.Kp = 2.0f;
	SteeringPID.Ki = 0.0f;
	SteeringPID.Kd = 0.5f;
	SteeringPID.OutputMin = -1.0f;
	SteeringPID.OutputMax = 1.0f;

	SpeedPID.Kp = 0.03f;
	SpeedPID.Ki = 0.003f;
	SpeedPID.Kd = 0.015f;
	SpeedPID.OutputMin = -1.0f;
	SpeedPID.OutputMax = 1.0f;
}

void UTurboAction_FollowPath::Start(bool bFirstTime)
{
	Super::Start(bFirstTime);

	AIController = GetTypedOuter<ATurboAIController>();
	if (AIController.IsValid())
	{
		Vehicle = AIController->GetVehicle();
		RacingSplineActor = AIController->GetRacingSplineActor();
		SpeedProfile.Calculate(RacingSplineActor.Get(), Vehicle.Get());
	}

	SteeringPID.Reset();
	SpeedPID.Reset();
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
	if (!Vehicle.IsValid() || !RacingSplineActor.IsValid() || !AIController.IsValid())
	{
		return;
	}

	Vehicle->SetSteeringInput(CalculateSteering(GetTargetPoint(), DeltaTime));
	ApplySpeedControl(DeltaTime);
}

// =============================================================================
// STEERING CONTROL
// =============================================================================

USplineComponent* UTurboAction_FollowPath::GetSpline() const
{
	return RacingSplineActor.IsValid() ? RacingSplineActor->GetSplineComponent() : nullptr;
}

float UTurboAction_FollowPath::GetLookaheadDistance() const
{
	if (!Vehicle.IsValid())
	{
		return MinLookaheadDistance;
	}

	const float SpeedCmPerSec = FMath::Abs(Vehicle->GetForwardSpeed());
	const float Lookahead = SpeedCmPerSec * LookaheadSpeedFactor;

	return FMath::Clamp(Lookahead, MinLookaheadDistance, MaxLookaheadDistance);
}

FVector UTurboAction_FollowPath::GetTargetPoint()
{
	if (!RacingSplineActor.IsValid() || !AIController.IsValid())
	{
		return Vehicle.IsValid() ? Vehicle->GetActorLocation() : FVector::ZeroVector;
	}

	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float SplineLength = RacingSplineActor->GetSplineLength();
	const float LookaheadDist = GetLookaheadDistance();
	float TargetDistance = CurrentDistance + LookaheadDist;

	if (RacingSplineActor->IsClosedLoop() && TargetDistance >= SplineLength)
	{
		TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
	}
	else
	{
		TargetDistance = FMath::Min(TargetDistance, SplineLength);
	}

	return RacingSplineActor->GetLocationAtDistance(TargetDistance);
}

float UTurboAction_FollowPath::CalculateSteering(const FVector& TargetPoint, float DeltaTime)
{
	if (!Vehicle.IsValid())
	{
		return 0.0f;
	}

	const FVector VehicleLocation = Vehicle->GetActorLocation();
	const FVector VehicleRight = Vehicle->GetActorRightVector();
	const FVector ToTarget = (TargetPoint - VehicleLocation).GetSafeNormal();

	const float LateralError = FVector::DotProduct(ToTarget, VehicleRight);

	return SteeringPID.Update(LateralError, DeltaTime);
}

// =============================================================================
// SPEED CONTROL
// =============================================================================

void UTurboAction_FollowPath::ApplySpeedControl(float DeltaTime)
{
	if (!Vehicle.IsValid() || !AIController.IsValid()) return;

	const float CurrentSpeedCms = FMath::Abs(Vehicle->GetForwardSpeed());
	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float TargetSpeedCms = GetTargetSpeedAtDistance(CurrentDistance);

	const float SpeedError = TargetSpeedCms - CurrentSpeedCms;

	float FinalThrottle = 0.0f;
	float FinalBrake = 0.0f;

	if (FMath::Abs(SpeedError) < CoastingThresholdCms)
	{
		FinalThrottle = CoastThrottleInput;
	}
	else
	{
		const float PIDOutput = SpeedPID.Update(SpeedError, DeltaTime);

		if (PIDOutput > 0.0f)
		{
			FinalThrottle = PIDOutput;
		}
		else
		{
			FinalBrake = -PIDOutput;
		}
	}

	Vehicle->SetThrottleInput(FinalThrottle);
	Vehicle->SetBrakeInput(FinalBrake);
	Vehicle->SetHandbrakeInput(false);
}

float UTurboAction_FollowPath::GetTargetSpeedAtDistance(float Distance) const
{
	if (!RacingSplineActor.IsValid() || !Vehicle.IsValid())
	{
		return Vehicle.IsValid() ? Vehicle->MaxSpeedCms : 0.0f;
	}

	if (!SpeedProfile.IsReady())
	{
		return Vehicle->MaxSpeedCms;
	}

	return SpeedProfile.GetTargetSpeed(
		Distance,
		RacingSplineActor->GetSplineLength(),
		RacingSplineActor->IsClosedLoop());
}

