// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboAIController.h"
#include "AI/TurboAIVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "AI/TurboActionStack.h"

UTurboAction_FollowPath::UTurboAction_FollowPath()
{
    ActionName = TEXT("FollowPath");
    ActionTag = TurboGameplayTags::Action_FollowPath;
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
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
    if (!Vehicle.IsValid() || !RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return;
    }

    FVector TargetPoint = GetTargetPoint();
    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    ApplySpeedControl();
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

    float SpeedCmPerSec = FMath::Abs(Vehicle->GetForwardSpeed());
    float Lookahead = SpeedCmPerSec * LookaheadSpeedFactor;

    return FMath::Clamp(Lookahead, MinLookaheadDistance, MaxLookaheadDistance);
}

FVector UTurboAction_FollowPath::GetTargetPoint()
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !AIController.IsValid())
    {
        return Vehicle.IsValid() ? Vehicle->GetActorLocation() : FVector::ZeroVector;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float SplineLength = Spline->GetSplineLength();
    float LookaheadDist = GetLookaheadDistance();
    float TargetDistance = CurrentDistance + LookaheadDist;

    // Wrap distance for closed loops
    if (Spline->IsClosedLoop() && TargetDistance >= SplineLength)
    {
        TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
    }
    else
    {
        TargetDistance = FMath::Min(TargetDistance, SplineLength);
    }

    // Get point on racing line or centerline
    if (RacingSplineActor->IsRacingLineReady())
    {
        return RacingSplineActor->GetPointOnRacingLine(TargetDistance);
    }

    return Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
}

float UTurboAction_FollowPath::CalculateSteering(const FVector& TargetPoint)
{
    if (!Vehicle.IsValid())
    {
        return 0.0f;
    }

    FVector VehicleLocation = Vehicle->GetActorLocation();
    FVector VehicleRight = Vehicle->GetActorRightVector();
    FVector ToTarget = (TargetPoint - VehicleLocation).GetSafeNormal();

    float DotRight = FVector::DotProduct(ToTarget, VehicleRight);

    return FMath::Clamp(DotRight * SteeringGain, -1.0f, 1.0f);
}

// =============================================================================
// SPEED CONTROL
// =============================================================================

void UTurboAction_FollowPath::ApplySpeedControl()
{
    if (!Vehicle.IsValid() || !AIController.IsValid()) return;

    const float CurrentSpeedCms = FMath::Abs(Vehicle->GetForwardSpeed());
    const float CurrentDistance = AIController->GetCurrentSplineDistance();
    const float TargetSpeedCms = GetTargetSpeedAtDistance(CurrentDistance);

    // Convert to km/h for the P-controller (keeps existing tuning values valid)
    const float CurrentSpeed = CurrentSpeedCms * 0.036f;
    const float TargetSpeed = TargetSpeedCms * 0.036f;
    const float SpeedError = TargetSpeed - CurrentSpeed;

    float FinalThrottle = 0.0f;
    float FinalBrake = 0.0f;

    if (SpeedError > CoastingThresholdKmh)
    {
        FinalThrottle = FMath::Clamp(SpeedError * ThrottleProportionalGain, MinThrottleInput, MaxThrottleInput);
    }
    else if (SpeedError < -CoastingThresholdKmh)
    {
        FinalBrake = FMath::Clamp(-SpeedError * BrakeProportionalGain, MinBrakeInput, MaxBrakeInput);
    }
    else
    {
        FinalThrottle = CoastThrottleInput;
    }

    Vehicle->SetThrottleInput(FinalThrottle);
    Vehicle->SetBrakeInput(FinalBrake);
    Vehicle->SetHandbrakeInput(false);
}

// =============================================================================
// SPEED PROFILE
// =============================================================================

void UTurboAction_FollowPath::CalculateSpeedProfile()
{
    bSpeedProfileReady = false;

    if (!RacingSplineActor.IsValid() || !Vehicle.IsValid()) return;

    USplineComponent* Spline = GetSpline();
    if (!Spline) return;

    const float SplineLength = Spline->GetSplineLength();
    const int32 NumSamples = FMath::CeilToInt(SplineLength / SpeedProfileSampleInterval);

    if (NumSamples <= 0) return;

    const float MaxSpeed = Vehicle->MaxSpeedCms;
    const float Grip = Vehicle->LateralGripCms2;
    const float BrakeDecel = Vehicle->BrakeDecelerationCms2;
    const float Accel = Vehicle->AccelerationCms2;

    SpeedProfile.SetNum(NumSamples);

    // ---- Pass 1: Cornering speed limits ----
    // v = sqrt(grip * radius) = sqrt(grip / curvature)

    for (int32 i = 0; i < NumSamples; i++)
    {
        const float Dist = i * SpeedProfileSampleInterval;
        const float Curvature = RacingSplineActor->GetCurvatureAtDistance(
            Dist, SpeedCurvatureSampleRange);

        if (Curvature > KINDA_SMALL_NUMBER)
        {
            const float CorneringSpeed = FMath::Sqrt(Grip / Curvature)
                * CorneringSpeedSafetyFactor;
            SpeedProfile[i] = FMath::Min(MaxSpeed, CorneringSpeed);
        }
        else
        {
            SpeedProfile[i] = MaxSpeed;
        }
    }

    // ---- Pass 2: Braking pass (reverse) ----
    // Walking backward: if the next point is slower, propagate the braking constraint
    // v = sqrt(v_next² + 2 * brakeDecel * distance)

    const bool bClosedLoop = Spline->IsClosedLoop();
    const float Ds = SpeedProfileSampleInterval;

    if (bClosedLoop)
    {
        // Two full reverse laps to let braking zones propagate across the start/finish
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
    // Walking forward: coming out of a corner, can only accelerate so fast
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
        USplineComponent* Spline = GetSpline();
        if (Spline)
        {
            const float SplineLength = Spline->GetSplineLength();
            if (Spline->IsClosedLoop())
            {
                Distance = FMath::Fmod(Distance, SplineLength);
                if (Distance < 0.0f) Distance += SplineLength;
            }
            else
            {
                Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);
            }
        }
    }

    const float IndexFloat = Distance / SpeedProfileSampleInterval;
    const int32 Index = FMath::Clamp(FMath::FloorToInt(IndexFloat), 0, SpeedProfile.Num() - 1);
    const int32 NextIndex = (Index + 1) % SpeedProfile.Num();
    const float Alpha = IndexFloat - FMath::FloorToInt(IndexFloat);

    return FMath::Lerp(SpeedProfile[Index], SpeedProfile[NextIndex], Alpha);
}

