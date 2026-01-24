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
        Vehicle = Cast<ATurboVehicle>(AIController->GetPawn());
        RacingSplineActor = AIController->GetRacingSplineActor();
    }

    if (bFirstTime && Vehicle.IsValid() && GetSpline())
    {
        FVector VehicleLocation = Vehicle->GetActorLocation();
        CurrentSplineDistance = GetSpline()->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
    }
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
    if (!Vehicle.IsValid() || !RacingSplineActor.IsValid())
    {
        return;
    }

    UpdateSplineDistance();

    FVector TargetPoint = GetTargetPoint();

    if (bDrawDebug)
    {
        UWorld* World = Vehicle->GetWorld();
        FVector VehicleLocation = Vehicle->GetActorLocation();
        USplineComponent* Spline = GetSpline();

        // Target point (green)
        DrawDebugSphere(World, TargetPoint, 50.0f, 12, FColor::Green, false, 0.0f);

        // Line to target (yellow)
        DrawDebugLine(World, VehicleLocation, TargetPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

        // Vehicle forward (red)
        FVector ForwardEnd = VehicleLocation + Vehicle->GetActorForwardVector() * 500.0f;
        DrawDebugLine(World, VehicleLocation, ForwardEnd, FColor::Red, false, 0.0f, 0, 3.0f);

        // Current spline position (blue)
        FVector CurrentSplinePoint = Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
        DrawDebugSphere(World, CurrentSplinePoint, 30.0f, 8, FColor::Blue, false, 0.0f);

        // Visualize curvature detection range
        if (bUseCurvatureSpeedControl)
        {
            float SplineLength = Spline->GetSplineLength();
            float MaxCurvature = 0.0f;
            float MaxCurvatureDistance = CurrentSplineDistance;

            // Draw curvature samples and find max
            for (float Dist = CurrentSplineDistance; Dist < CurrentSplineDistance + CornerDetectionDistance; Dist += 100.0f)
            {
                float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
                float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);

                FVector SamplePoint = Spline->GetLocationAtDistanceAlongSpline(WrappedDist, ESplineCoordinateSpace::World);

                // Color based on curvature: green (straight) -> red (sharp turn)
                FColor CurvatureColor = FColor::MakeRedToGreenColorFromScalar(1.0f - Curvature);
                DrawDebugPoint(World, SamplePoint + FVector(0, 0, 50), 10.0f, CurvatureColor, false, 0.0f);

                if (Curvature > MaxCurvature)
                {
                    MaxCurvature = Curvature;
                    MaxCurvatureDistance = WrappedDist;
                }
            }

            // Mark the sharpest corner ahead (orange sphere)
            if (MaxCurvature > 0.05f)
            {
                FVector MaxCurvaturePoint = Spline->GetLocationAtDistanceAlongSpline(MaxCurvatureDistance, ESplineCoordinateSpace::World);
                DrawDebugSphere(World, MaxCurvaturePoint + FVector(0, 0, 100), 40.0f, 8, FColor::Orange, false, 0.0f);
            }
        }

        // On-screen debug info
        float CurrentSpeed = Vehicle->GetSpeedKmh();
        float CurrentTargetSpeed = CalculateTargetSpeed();
        float MaxCurvature = FindMaxCurvatureAhead();

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f / %.1f km/h"), CurrentSpeed, CurrentTargetSpeed));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,
            FString::Printf(TEXT("Lookahead: %.0f cm"), GetLookaheadDistance()));
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::White,
            FString::Printf(TEXT("Max Curvature Ahead: %.3f"), MaxCurvature));
    }

    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    ApplySpeedControl();

    /*
    if (bDrawDebug && bUseCurvatureSpeedControl)
    {
        USplineComponent* Spline = GetSpline();
        float SplineLength = Spline->GetSplineLength();

        UE_LOG(LogTemp, Warning, TEXT("=== Curvature Debug ==="));
        UE_LOG(LogTemp, Warning, TEXT("Current Distance: %.0f, Spline Length: %.0f"), CurrentSplineDistance, SplineLength);

        // Sample a few points and log their curvature
        for (float Dist = CurrentSplineDistance; Dist < CurrentSplineDistance + CornerDetectionDistance; Dist += 500.0f)
        {
            float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
            float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);

            UE_LOG(LogTemp, Warning, TEXT("  Dist: %.0f -> Curvature: %.6f"), WrappedDist, Curvature);
        }

        UE_LOG(LogTemp, Warning, TEXT("Max Curvature Found: %.6f"), FindMaxCurvatureAhead());
    }
    */
}

USplineComponent* UTurboAction_FollowPath::GetSpline() const
{
    return RacingSplineActor.IsValid() ? RacingSplineActor->GetSplineComponent() : nullptr;
}

void UTurboAction_FollowPath::UpdateSplineDistance()
{
    USplineComponent* Spline = GetSpline();
    if (!Vehicle.IsValid() || !Spline)
    {
        return;
    }

    FVector VehicleLocation = Vehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
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

FVector UTurboAction_FollowPath::GetTargetPoint() const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline)
    {
        return Vehicle.IsValid() ? Vehicle->GetActorLocation() : FVector::ZeroVector;
    }

    float SplineLength = Spline->GetSplineLength();
    float LookaheadDist = GetLookaheadDistance();
    float TargetDistance = CurrentSplineDistance + LookaheadDist;

    if (Spline->IsClosedLoop() && TargetDistance >= SplineLength)
    {
        TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
    }
    else
    {
        TargetDistance = FMath::Min(TargetDistance, SplineLength);
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

    float SteeringInput = FMath::Clamp(DotRight * 2.0f, -1.0f, 1.0f);

    return SteeringInput;
}

float UTurboAction_FollowPath::FindMaxCurvatureAhead() const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return 0.0f;
    }

    float SplineLength = Spline->GetSplineLength();
    float MaxCurvature = 0.0f;

    // Sample curvature at regular intervals ahead
    for (float Dist = CurrentSplineDistance; Dist < CurrentSplineDistance + CornerDetectionDistance; Dist += 100.0f)
    {
        float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);
        MaxCurvature = FMath::Max(MaxCurvature, Curvature);
    }

    return MaxCurvature;
}

float UTurboAction_FollowPath::CalculateTargetSpeed() const
{
    if (!bUseCurvatureSpeedControl)
    {
        return TargetSpeedKmh;
    }

    float MaxCurvature = FindMaxCurvatureAhead();

    // Curvature 0 = straight = MaxSpeed
    // Curvature 1 = hairpin = MinCornerSpeed
    float SpeedReduction = MaxCurvature * CurvatureBrakingSensitivity;
    float DesiredSpeed = MaxSpeedKmh - SpeedReduction;

    return FMath::Clamp(DesiredSpeed, MinCornerSpeedKmh, MaxSpeedKmh);
}

void UTurboAction_FollowPath::ApplySpeedControl()
{
    if (!Vehicle.IsValid())
    {
        return;
    }

    float CurrentSpeed = Vehicle->GetSpeedKmh();
    float DesiredSpeed = CalculateTargetSpeed();
    float SpeedError = DesiredSpeed - CurrentSpeed;

    if (SpeedError > 0.0f)
    {
        // Accelerate
        Vehicle->SetThrottleInput(FMath::Clamp(SpeedError / 20.0f, 0.0f, 1.0f));
        Vehicle->SetBrakeInput(0.0f);
    }
    else
    {
        // Brake - more aggressive braking for larger errors
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(FMath::Clamp(-SpeedError / 30.0f, 0.0f, 1.0f));
    }

    Vehicle->SetHandbrakeInput(false);
}