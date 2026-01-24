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

    // Log what we have
    UE_LOG(LogTemp, Warning, TEXT("FollowPath Started - Vehicle: %s, Spline: %s"),
        Vehicle.IsValid() ? TEXT("Valid") : TEXT("INVALID"),
        RacingSplineActor.IsValid() ? TEXT("Valid") : TEXT("INVALID"));

    if (bFirstTime && Vehicle.IsValid() && GetSpline())
    {
        // Initialize spline distance to closest point
        FVector VehicleLocation = Vehicle->GetActorLocation();
        float ClosestDist = GetSpline()->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
        CurrentSplineDistance = ClosestDist;

        UE_LOG(LogTemp, Warning, TEXT("Initial spline distance: %f"), CurrentSplineDistance);
    }
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
    if (!Vehicle.IsValid() || !RacingSplineActor.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FollowPath Update - Missing references!"));
        return;
    }

    UpdateSplineDistance();

    FVector TargetPoint = GetTargetPoint();

    // Debug visualization
    if (bDrawDebug)
    {
        FVector VehicleLocation = Vehicle->GetActorLocation();

        // Draw target point (green sphere)
        DrawDebugSphere(Vehicle->GetWorld(), TargetPoint, 50.0f, 12, FColor::Green, false, 0.0f);

        // Draw line from vehicle to target (yellow)
        DrawDebugLine(Vehicle->GetWorld(), VehicleLocation, TargetPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

        // Draw vehicle forward vector (red)
        FVector ForwardEnd = VehicleLocation + Vehicle->GetActorForwardVector() * 500.0f;
        DrawDebugLine(Vehicle->GetWorld(), VehicleLocation, ForwardEnd, FColor::Red, false, 0.0f, 0, 3.0f);

        // Draw current spline position (blue sphere)
        FVector CurrentSplinePoint = GetSpline()->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
        DrawDebugSphere(Vehicle->GetWorld(), CurrentSplinePoint, 30.0f, 8, FColor::Blue, false, 0.0f);
    }

    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    ApplySimpleSpeedControl();
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

    // Simple approach: find closest point on spline
    FVector VehicleLocation = Vehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

FVector UTurboAction_FollowPath::GetTargetPoint() const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline)
    {
        return Vehicle.IsValid() ? Vehicle->GetActorLocation() : FVector::ZeroVector;
    }

    float SplineLength = Spline->GetSplineLength();
    float TargetDistance = CurrentSplineDistance + LookaheadDistance;

    // Handle wrap-around for closed splines
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

    // Direction to target
    FVector ToTarget = (TargetPoint - VehicleLocation).GetSafeNormal();

    // How far right is the target? Positive = turn right, negative = turn left
    float DotRight = FVector::DotProduct(ToTarget, VehicleRight);

    // Simple proportional steering
    float SteeringInput = FMath::Clamp(DotRight * 2.0f, -1.0f, 1.0f);

    if (bDrawDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("Steering: DotRight=%.3f, Input=%.3f"), DotRight, SteeringInput);
    }

    return SteeringInput;
}

void UTurboAction_FollowPath::ApplySimpleSpeedControl()
{
    if (!Vehicle.IsValid())
    {
        return;
    }

    float CurrentSpeed = Vehicle->GetSpeedKmh();
    float SpeedError = TargetSpeedKmh - CurrentSpeed;

    if (SpeedError > 0.0f)
    {
        // Need to go faster
        Vehicle->SetThrottleInput(FMath::Clamp(SpeedError / 20.0f, 0.0f, 1.0f));
        Vehicle->SetBrakeInput(0.0f);
    }
    else
    {
        // Need to slow down
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(FMath::Clamp(-SpeedError / 20.0f, 0.0f, 1.0f));
    }

    Vehicle->SetHandbrakeInput(false);
}