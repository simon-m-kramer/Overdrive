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

    // Action is owned by ActionStack and ActionStack is owned by AIController. GetTypedOuter goes through the whole outer chain.
    AIController = GetTypedOuter<ATurboAIController>();
    if (AIController.IsValid())
    {
        Vehicle = AIController->GetVehicle();
        RacingSplineActor = AIController->GetRacingSplineActor();
    }
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

    return FMath::Clamp(DotRight * 2.0f, -1.0f, 1.0f);
}

// =============================================================================
// SPEED CONTROL
// =============================================================================

float UTurboAction_FollowPath::FindTargetSpeedAhead() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return MaxSpeedKmh;
    }

    float LowestRequiredSpeed = MaxSpeedKmh;
    float CurrentDistance = AIController->GetCurrentSplineDistance();

    for (float Ahead = 0.0f; Ahead < CornerScanDistance; Ahead += CornerScanInterval)
    {
        float ScanDist = CurrentDistance + Ahead;
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(ScanDist, SpeedCurvatureSampleRange);

        if (Curvature > KINDA_SMALL_NUMBER)
        {
            // Physics-based speed: v = sqrt(grip / curvature)
            // Result is in cm/s, convert to km/h
            float RequiredSpeedCmPerSec = FMath::Sqrt(GripFactor / Curvature);
            float RequiredSpeedKmh = RequiredSpeedCmPerSec * 0.036f; // cm/s to km/h

            // Adjust for distance - can carry more speed if corner is far
            float DistanceFactor = Ahead / CornerScanDistance;
            float AdjustedSpeed = RequiredSpeedKmh + (DistanceFactor * DistanceSpeedBuffer);

            LowestRequiredSpeed = FMath::Min(LowestRequiredSpeed, AdjustedSpeed);
        }
    }

    return FMath::Clamp(LowestRequiredSpeed, MinCornerSpeedKmh, MaxSpeedKmh);
}

void UTurboAction_FollowPath::ApplySpeedControl()
{
    if (!Vehicle.IsValid())
    {
        return;
    }

    const float CurrentSpeed = Vehicle->GetSpeedKmh();
    const float TargetSpeed = FindTargetSpeedAhead();
    const float SpeedError = TargetSpeed - CurrentSpeed;

    float FinalThrottle = 0.0f;
    float FinalBrake = 0.0f;

    if (SpeedError > CoastingThresholdKmh)
    {
        // P-Loop for Throttle
        FinalThrottle = FMath::Clamp(SpeedError * ThrottleProportionalGain, MinThrottleInput, 1.0f);
    }
    else if (SpeedError < -CoastingThresholdKmh)
    {
        // P-Loop for Braking (Negative SpeedError makes a positive BrakeInput)
        FinalBrake = FMath::Clamp(-SpeedError * BrakeProportionalGain, MinBrakeInput, MaxBrakeInput);
    }
    else
    {
        // Maintenance Mode
        FinalThrottle = CoastThrottleInput;
    }

    Vehicle->SetThrottleInput(FinalThrottle);
    Vehicle->SetBrakeInput(FinalBrake);
    Vehicle->SetHandbrakeInput(false);
}

