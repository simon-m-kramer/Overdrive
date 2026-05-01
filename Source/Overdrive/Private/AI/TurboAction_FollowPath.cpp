// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboActionStack.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Framework/TurboDrivingProfile.h"
#include "DrawDebugHelpers.h"

UTurboAction_FollowPath::UTurboAction_FollowPath()
{
	ActionName = TEXT("FollowPath");
	ActionTag = TurboGameplayTags::Action_FollowPath;

	SteeringPID.Kp = 3.0f;
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

	if (bFirstTime)
	{
		AIController = GetTypedOuter<ATurboAIController>();
		if (AIController.IsValid())
		{
			Vehicle = AIController->GetVehicle();
			RacingSpline = AIController->GetRacingSplineActor();
		}
	}

	SteeringPID.Reset();
	SpeedPID.Reset();
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
	if (!Vehicle.IsValid() || !RacingSpline.IsValid() || !AIController.IsValid())
	{
		return;
	}

	const FVector TargetPoint = GetTargetPoint();
	const float SteeringInput = CalculateSteering(TargetPoint, DeltaTime);
	Vehicle->SetSteeringInput(SteeringInput);

	ApplySpeedControl(DeltaTime);
}

// =============================================================================
// STEERING CONTROL
// =============================================================================

USplineComponent* UTurboAction_FollowPath::GetSpline() const
{
	return RacingSpline.IsValid() ? RacingSpline->GetSplineComponent() : nullptr;
}

FVector UTurboAction_FollowPath::GetTargetPoint()
{
	if (!RacingSpline.IsValid() || !AIController.IsValid())
	{
		return Vehicle.IsValid() ? Vehicle->GetActorLocation() : FVector::ZeroVector;
	}

	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float SplineLength = RacingSpline->GetSplineLength();
	const float LookaheadDist = GetSteeringLookahead();
	float TargetDistance = CurrentDistance + LookaheadDist;

	if (RacingSpline->IsClosedLoop() && TargetDistance >= SplineLength)
	{
		TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
	}
	else
	{
		TargetDistance = FMath::Min(TargetDistance, SplineLength);
	}

	return RacingSpline->GetLocationAtDistance(TargetDistance);
}

FVector UTurboAction_FollowPath::GetTargetPointWithLateralOffset(float LateralOffset)
{
	// namespace to make sure we call this class' version, not the child's version
	const FVector BaseTarget = UTurboAction_FollowPath::GetTargetPoint();

	if (FMath::IsNearlyZero(LateralOffset) || !AIController.IsValid())
	{
		return BaseTarget;
	}

	USplineComponent* Spline = GetSpline();
	if (!Spline)
	{
		return BaseTarget;
	}

	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float LookaheadDist = GetSteeringLookahead();
	float TargetDistance = CurrentDistance + LookaheadDist;

	if (Spline->IsClosedLoop())
	{
		const float SplineLength = Spline->GetSplineLength();
		TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
		if (TargetDistance < 0.0f) TargetDistance += SplineLength;
	}

	const FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	const FVector Right = FVector::CrossProduct(Up, Tangent).GetSafeNormal();

	return BaseTarget + (Right * LateralOffset);
}

float UTurboAction_FollowPath::CalculateSteering(const FVector& TargetPoint, float DeltaTime)
{
	if (!Vehicle.IsValid())
	{
		return 0.0f;
	}

	const FTransform VehicleTransform = Vehicle->GetActorTransform();
	const FVector LocalTarget = VehicleTransform.InverseTransformPosition(TargetPoint);
	const float HeadingError = FMath::Atan2(LocalTarget.Y, LocalTarget.X);

	return SteeringPID.Update(HeadingError, DeltaTime);
}

// =============================================================================
// SPEED CONTROL
// =============================================================================

void UTurboAction_FollowPath::ApplySpeedControl(float DeltaTime)
{
	if (!Vehicle.IsValid() || !AIController.IsValid()) return;

	const float CurrentSpeedCms = FMath::Abs(Vehicle->GetForwardSpeed());
	const float CurrentDistance = AIController->GetCurrentSplineDistance();
	const float SpeedLookahead = GetSpeedLookahead();
	const float TargetSpeedCms = AIController->GetTargetSpeedAtDistance(CurrentDistance + SpeedLookahead);
	const float SpeedError = TargetSpeedCms - CurrentSpeedCms;

	const float PIDOutput = SpeedPID.Update(SpeedError, DeltaTime);

	float FinalThrottle = 0.0f;
	float FinalBrake = 0.0f;

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

// =============================================================================
// LOOK AHEAD
// =============================================================================

float UTurboAction_FollowPath::GetSteeringLookahead() const
{
	return ComputeLookahead(SteeringLookaheadFactor, MinSteeringLookahead, MaxSteeringLookahead);
}

float UTurboAction_FollowPath::GetSpeedLookahead() const
{
	return ComputeLookahead(SpeedLookaheadFactor, MinSpeedLookahead, MaxSpeedLookahead);
}

float UTurboAction_FollowPath::ComputeLookahead(float Factor, float MinDistance, float MaxDistance) const
{
	if (!Vehicle.IsValid())
	{
		return MinDistance;
	}

	const float Speed = FMath::Abs(Vehicle->GetForwardSpeed());
	return FMath::Clamp(Speed * Factor, MinDistance, MaxDistance);
}


