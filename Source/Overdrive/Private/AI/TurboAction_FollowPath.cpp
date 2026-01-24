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

        // Centerline target for comparison (cyan) - shows where we'd aim without racing line
        if (bUseRacingLineOffset)
        {
            float LookaheadDist = GetLookaheadDistance();
            float SplineLength = Spline->GetSplineLength();
            float TargetDistance = CurrentSplineDistance + LookaheadDist;
            if (Spline->IsClosedLoop())
            {
                TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
            }
            FVector CenterlinePoint = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
            DrawDebugSphere(World, CenterlinePoint, 30.0f, 8, FColor::Cyan, false, 0.0f);

            // Draw racing line offset as a line from centerline to actual target
            DrawDebugLine(World, CenterlinePoint, TargetPoint, FColor::Magenta, false, 0.0f, 0, 2.0f);

            // Find and mark the apex
            float ApexDist = FindApexDistance(CurrentSplineDistance, RacingLineLookahead);
            if (ApexDist > 0.0f)
            {
                FVector ApexPoint = Spline->GetLocationAtDistanceAlongSpline(ApexDist, ESplineCoordinateSpace::World);
                DrawDebugSphere(World, ApexPoint + FVector(0, 0, 100), 60.0f, 8, FColor::Orange, false, 0.0f);
            }
        }

        // On-screen debug info
        float CurrentSpeed = Vehicle->GetSpeedKmh();
        float CurrentTargetSpeed = CalculateTargetSpeed();
        float MaxCurvature = FindMaxCurvatureAhead();
        float CurrentOffset = CalculateRacingLineOffset(CurrentSplineDistance + GetLookaheadDistance());

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f / %.1f km/h"), CurrentSpeed, CurrentTargetSpeed));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,
            FString::Printf(TEXT("Lookahead: %.0f cm"), GetLookaheadDistance()));
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::White,
            FString::Printf(TEXT("Max Curvature: %.3f"), MaxCurvature));
        GEngine->AddOnScreenDebugMessage(4, 0.0f, FColor::White,
            FString::Printf(TEXT("Racing Line Offset: %.1f cm"), CurrentOffset));
    }

    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    ApplySpeedControl();
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

float UTurboAction_FollowPath::FindApexDistance(float StartDistance, float SearchRange) const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return -1.0f;
    }

    float SplineLength = Spline->GetSplineLength();
    float MaxCurvature = 0.0f;
    float ApexDistance = -1.0f;

    for (float Dist = StartDistance; Dist < StartDistance + SearchRange; Dist += 100.0f)
    {
        float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);

        if (Curvature > MaxCurvature && Curvature > RacingLineMinCurvature)
        {
            MaxCurvature = Curvature;
            ApexDistance = WrappedDist;
        }
    }

    return ApexDistance;
}

float UTurboAction_FollowPath::CalculateRacingLineOffset(float AtDistance) const
{
    if (!bUseRacingLineOffset)
    {
        return 0.0f;
    }

    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return 0.0f;
    }

    float SplineLength = Spline->GetSplineLength();

    // Find the apex (sharpest point) ahead
    float ApexDistance = FindApexDistance(CurrentSplineDistance, RacingLineLookahead);

    // No significant corner ahead
    if (ApexDistance < 0.0f)
    {
        return 0.0f;
    }

    // Get curvature at apex to scale the offset
    float ApexCurvature = RacingSplineActor->GetCurvatureAtDistance(ApexDistance, CurvatureSampleRange);

    // Get turn direction at apex
    float TurnSign = RacingSplineActor->GetTurnSign(ApexDistance);

    // Calculate distance to apex
    float DistanceToApex = ApexDistance - CurrentSplineDistance;
    if (DistanceToApex < 0.0f)
    {
        DistanceToApex += SplineLength;
    }

    // Normalize distance to 0-1 range (1 = far from apex, 0 = at apex)
    float NormalizedDist = FMath::Clamp(DistanceToApex / RacingLineLookahead, 0.0f, 1.0f);

    // Racing line phase using cosine:
    // NormalizedDist = 1.0 (far): cos(0) = 1.0 -> outside of turn (wide entry)
    // NormalizedDist = 0.5 (mid): cos(PI/2) = 0.0 -> on centerline
    // NormalizedDist = 0.0 (apex): cos(PI) = -1.0 -> inside of turn (clip apex)
    float Phase = FMath::Cos(NormalizedDist * PI);

    // Scale offset by curvature (sharper corner = more offset)
    float OffsetMagnitude = ApexCurvature * MaxRacingLineOffset;

    // TurnSign: positive = right turn, negative = left turn
    // For a right turn: we want to be LEFT (negative) on entry, RIGHT (positive) at apex
    // Phase goes from 1 (entry) to -1 (apex)
    // So: Offset = -TurnSign * Phase * Magnitude
    return -TurnSign * Phase * OffsetMagnitude;
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

    // Get centerline point
    FVector CenterlinePoint = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);

    // Calculate racing line offset
    float Offset = CalculateRacingLineOffset(TargetDistance);

    if (FMath::Abs(Offset) < 1.0f)
    {
        return CenterlinePoint;
    }

    // Get the right vector at this spline point
    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    return CenterlinePoint + (Right * Offset);
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
        Vehicle->SetThrottleInput(FMath::Clamp(SpeedError / 20.0f, 0.0f, 1.0f));
        Vehicle->SetBrakeInput(0.0f);
    }
    else
    {
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(FMath::Clamp(-SpeedError / 30.0f, 0.0f, 1.0f));
    }

    Vehicle->SetHandbrakeInput(false);
}