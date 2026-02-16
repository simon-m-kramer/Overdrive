// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboAIController.h"
#include "AI/TurboAIVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "AI/TurboActionStack.h"

UTurboAction_FollowPath::UTurboAction_FollowPath()
{
	ActionName = TEXT("FollowPath");
	ActionTag = TurboGameplayTags::Action_FollowPath;

	SteeringPID.Kp = 2.0f;
	SteeringPID.Ki = 0.0f;
	SteeringPID.Kd = 0.5f;
	SteeringPID.OutputMin = -1.0f;
	SteeringPID.OutputMax = 1.0f;

	SpeedPID.Kp = 0.01f;
	SpeedPID.Ki = 0.001f;
	SpeedPID.Kd = 0.005f;
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
	}

	CalculateSpeedProfile();

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
// STEERING
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

	const float ProfileSpeed = GetTargetSpeedAtDistance(CurrentDistance);
	const float FollowLimit = GetFollowSpeedLimit();
	const float TargetSpeedCms = FMath::Min(ProfileSpeed, FollowLimit);

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

float UTurboAction_FollowPath::GetFollowSpeedLimit() const
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

	const float T = FMath::Clamp(
		(FollowReactionDistance - Distance) / (FollowReactionDistance - FollowMinDistance),
		0.0f, 1.0f);

	const float MatchSpeed = TheirSpeed - FollowSpeedMarginCms;
	const float OurMaxSpeed = Vehicle.IsValid() ? Vehicle->MaxSpeedCms : MAX_FLT;

	return FMath::Lerp(OurMaxSpeed, MatchSpeed, T);
}

// =============================================================================
// SPEED PROFILE
// =============================================================================

void UTurboAction_FollowPath::CalculateSpeedProfile()
{
	bSpeedProfileReady = false;

	if (!RacingSplineActor.IsValid() || !Vehicle.IsValid()) return;

	const float SplineLength = RacingSplineActor->GetSplineLength();
	const int32 NumSamples = FMath::CeilToInt(SplineLength / SpeedProfileSampleInterval);

	if (NumSamples <= 0) return;

	const float MaxSpeed = Vehicle->MaxSpeedCms;
	const float Grip = Vehicle->LateralGripCms2;
	const float BrakeDecel = Vehicle->BrakeDecelerationCms2;
	const float Accel = Vehicle->AccelerationCms2;

	SpeedProfile.SetNum(NumSamples);

	// ---- Pass 1: Cornering speed limits ----
	// v = sqrt(grip / curvature), with exit anticipation

	for (int32 i = 0; i < NumSamples; i++)
	{
		const float Dist = i * SpeedProfileSampleInterval;
		const float Curvature = RacingSplineActor->GetCurvatureAtDistance(
			Dist, SpeedCurvatureSampleRange);

		const float AheadCurvature = RacingSplineActor->GetCurvatureAtDistance(
			Dist + ExitLookahead, SpeedCurvatureSampleRange);

		const float EffectiveCurvature = (AheadCurvature < Curvature)
			? FMath::Lerp(Curvature, AheadCurvature, ExitAnticipation)
			: Curvature;

		if (EffectiveCurvature > KINDA_SMALL_NUMBER)
		{
			const float CorneringSpeed = FMath::Sqrt(Grip / EffectiveCurvature)
				* CorneringSpeedSafetyFactor;
			SpeedProfile[i] = FMath::Min(MaxSpeed, CorneringSpeed);
		}
		else
		{
			SpeedProfile[i] = MaxSpeed;
		}
	}

	// ---- Pass 2: Braking pass (reverse) ----
	// v = sqrt(v_next² + 2 * brakeDecel * distance)

	const bool bClosedLoop = RacingSplineActor->IsClosedLoop();
	const float Ds = SpeedProfileSampleInterval;

	if (bClosedLoop)
	{
		for (int32 Lap = 0; Lap < 2; Lap++)
		{
			for (int32 i = NumSamples - 1; i >= 0; i--)
			{
				const int32 NextIndex = (i + 1) % NumSamples;
				const float BrakeLimit = FMath::Sqrt(
					SpeedProfile[NextIndex] * SpeedProfile[NextIndex]
					+ 2.0f * BrakeDecel * Ds);
				SpeedProfile[i] = FMath::Min(SpeedProfile[i], BrakeLimit);
			}
		}
	}
	else
	{
		for (int32 i = NumSamples - 2; i >= 0; i--)
		{
			const float BrakeLimit = FMath::Sqrt(
				SpeedProfile[i + 1] * SpeedProfile[i + 1]
				+ 2.0f * BrakeDecel * Ds);
			SpeedProfile[i] = FMath::Min(SpeedProfile[i], BrakeLimit);
		}
	}

	// ---- Pass 3: Acceleration pass (forward) ----
	// v = sqrt(v_prev² + 2 * accel * distance)

	if (bClosedLoop)
	{
		for (int32 Lap = 0; Lap < 2; Lap++)
		{
			for (int32 i = 0; i < NumSamples; i++)
			{
				const int32 PrevIndex = (i - 1 + NumSamples) % NumSamples;
				const float AccelLimit = FMath::Sqrt(
					SpeedProfile[PrevIndex] * SpeedProfile[PrevIndex]
					+ 2.0f * Accel * Ds);
				SpeedProfile[i] = FMath::Min(SpeedProfile[i], AccelLimit);
			}
		}
	}
	else
	{
		for (int32 i = 1; i < NumSamples; i++)
		{
			const float AccelLimit = FMath::Sqrt(
				SpeedProfile[i - 1] * SpeedProfile[i - 1]
				+ 2.0f * Accel * Ds);
			SpeedProfile[i] = FMath::Min(SpeedProfile[i], AccelLimit);
		}
	}

	bSpeedProfileReady = true;
}

float UTurboAction_FollowPath::GetTargetSpeedAtDistance(float Distance) const
{
	if (!bSpeedProfileReady || SpeedProfile.Num() == 0 || !Vehicle.IsValid())
	{
		return Vehicle.IsValid() ? Vehicle->MaxSpeedCms : 0.0f;
	}

	if (RacingSplineActor.IsValid())
	{
		const float SplineLength = RacingSplineActor->GetSplineLength();

		if (RacingSplineActor->IsClosedLoop())
		{
			Distance = FMath::Fmod(Distance, SplineLength);
			if (Distance < 0.0f) Distance += SplineLength;
		}
		else
		{
			Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);
		}
	}

	const float IndexFloat = Distance / SpeedProfileSampleInterval;
	const int32 Index = FMath::Clamp(FMath::FloorToInt(IndexFloat), 0, SpeedProfile.Num() - 1);
	const int32 NextIndex = (Index + 1) % SpeedProfile.Num();
	const float Alpha = IndexFloat - FMath::FloorToInt(IndexFloat);

	return FMath::Lerp(SpeedProfile[Index], SpeedProfile[NextIndex], Alpha);
}


