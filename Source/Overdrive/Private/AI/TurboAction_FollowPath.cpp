// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

void UTurboAction_FollowPath::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    AIController = Cast<ATurboAIController>(GetOuter());
    if (AIController.IsValid())
    {
        Vehicle = AIController->GetControlledVehicle();
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

    if (bDrawDebug)
    {
        UWorld* World = Vehicle->GetWorld();
        FVector VehicleLocation = Vehicle->GetActorLocation();
        USplineComponent* Spline = GetSpline();
        float CurrentDistance = AIController->GetCurrentSplineDistance();

        // Target point (green)
        DrawDebugSphere(World, TargetPoint, 50.0f, 12, FColor::Green, false, 0.0f);

        // Line to target (yellow)
        DrawDebugLine(World, VehicleLocation, TargetPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

        // Vehicle forward (red)
        FVector ForwardEnd = VehicleLocation + Vehicle->GetActorForwardVector() * 500.0f;
        DrawDebugLine(World, VehicleLocation, ForwardEnd, FColor::Red, false, 0.0f, 0, 3.0f);

        // Current spline position (blue)
        FVector CurrentSplinePoint = Spline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        DrawDebugSphere(World, CurrentSplinePoint, 30.0f, 8, FColor::Blue, false, 0.0f);

        // DEBUG: Visualize corner scanning
        float WorstCurvature = 0.0f;
        float WorstDistance = 0.0f;
        float WorstRequiredSpeed = MaxSpeedKmh;

        for (float Ahead = 0.0f; Ahead < CornerScanDistance; Ahead += CornerScanInterval)
        {
            float ScanDist = CurrentDistance + Ahead;
            float Curvature = RacingSplineActor->GetCurvatureAtDistance(ScanDist, SpeedCurvatureSampleRange);
            FVector ScanPoint = Spline->GetLocationAtDistanceAlongSpline(ScanDist, ESplineCoordinateSpace::World) + FVector(0, 0, 50);

            // Color based on curvature: green = straight, red = sharp
            float CurvatureNormalized = FMath::Clamp(Curvature * 50000.0f, 0.0f, 1.0f);
            FColor ScanColor = FColor::MakeRedToGreenColorFromScalar(1.0f - CurvatureNormalized);
            DrawDebugPoint(World, ScanPoint, 10.0f, ScanColor, false, 0.0f);

            if (Curvature > WorstCurvature)
            {
                WorstCurvature = Curvature;
                WorstDistance = Ahead;
                if (Curvature > KINDA_SMALL_NUMBER)
                {
                    float SpeedCmPerSec = FMath::Sqrt(GripFactor / Curvature);
                    WorstRequiredSpeed = SpeedCmPerSec * 0.036f;
                }
            }
        }

        // Mark the sharpest corner found
        if (WorstCurvature > KINDA_SMALL_NUMBER)
        {
            FVector WorstPoint = Spline->GetLocationAtDistanceAlongSpline(CurrentDistance + WorstDistance, ESplineCoordinateSpace::World) + FVector(0, 0, 100);
            DrawDebugSphere(World, WorstPoint, 80.0f, 8, FColor::Magenta, false, 0.0f);
        }

        // On-screen info
        float CurrentSpeed = Vehicle->GetSpeedKmh();
        float TargetSpeed = FindTargetSpeedAhead();
        float SpeedError = TargetSpeed - CurrentSpeed;

        FString State = SpeedError > CoastingThresholdKmh ? TEXT("THROTTLE") :
            SpeedError < -CoastingThresholdKmh ? TEXT("BRAKE") : TEXT("COAST");

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f / %.1f km/h [%s]"), CurrentSpeed, TargetSpeed, *State));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Worst corner in %.0f cm, curvature: %.6f"), WorstDistance, WorstCurvature));
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Required speed at corner: %.1f km/h"), WorstRequiredSpeed));
        GEngine->AddOnScreenDebugMessage(4, 0.0f, FColor::Green,
            FString::Printf(TEXT("Speed error: %.1f km/h"), SpeedError));
    }

    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    ApplySpeedControl();
}

// =============================================================================
// CORE METHODS
// =============================================================================

USplineComponent* UTurboAction_FollowPath::GetSpline() const
{
    return RacingSplineActor.IsValid() ? RacingSplineActor->GetSplineComponent() : nullptr;
}

float UTurboAction_FollowPath::GetLookaheadDistance() const
{
    if (!bUseSpeedDependentLookahead)
    {
        return FixedLookaheadDistance;
    }

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
    if (bUseRacingLine && RacingSplineActor->IsRacingLineReady())
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

    float CurrentSpeed = Vehicle->GetSpeedKmh();
    float TargetSpeed = FindTargetSpeedAhead();
    float SpeedError = TargetSpeed - CurrentSpeed;

    if (SpeedError > CoastingThresholdKmh)
    {
        // Too slow - accelerate
        float ThrottleInput = FMath::Clamp(SpeedError * ThrottleProportionalGain, MinThrottleInput, 1.0f);
        Vehicle->SetThrottleInput(ThrottleInput);
        Vehicle->SetBrakeInput(0.0f);
    }
    else if (SpeedError < -CoastingThresholdKmh)
    {
        // Too fast - brake
        float BrakeInput = FMath::Clamp(-SpeedError * BrakeProportionalGain, MinBrakeInput, MaxBrakeInput);
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(BrakeInput);
    }
    else
    {
        // Coast - maintain speed
        Vehicle->SetThrottleInput(CoastThrottleInput);
        Vehicle->SetBrakeInput(0.0f);
    }

    Vehicle->SetHandbrakeInput(false);
}