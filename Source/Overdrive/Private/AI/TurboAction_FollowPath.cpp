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

        // Initialize smoothing state
        SmoothedRacingLineOffset = 0.0f;
        PreviousTargetOffset = 0.0f;
    }
}

void UTurboAction_FollowPath::Update(float DeltaTime)
{
    if (!Vehicle.IsValid() || !RacingSplineActor.IsValid())
    {
        return;
    }

    UpdateSplineDistance();

    FVector TargetPoint = GetTargetPoint(DeltaTime);

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

            DrawDebugLine(World, CenterlinePoint, TargetPoint, FColor::Magenta, false, 0.0f, 0, 2.0f);

            // Find and mark corners
            FCornerInfo NextCorner = FindNextCorner(CurrentSplineDistance, RacingLineLookahead);
            if (NextCorner.bIsValid)
            {
                FVector ApexPoint = Spline->GetLocationAtDistanceAlongSpline(NextCorner.ApexDistance, ESplineCoordinateSpace::World);
                DrawDebugSphere(World, ApexPoint + FVector(0, 0, 100), 60.0f, 8, FColor::Orange, false, 0.0f);

                // Also check for corner after this one
                FCornerInfo FollowingCorner = FindCornerAfterStraight(NextCorner.ApexDistance, RacingLineLookahead);
                if (FollowingCorner.bIsValid)
                {
                    FVector FollowingApexPoint = Spline->GetLocationAtDistanceAlongSpline(FollowingCorner.ApexDistance, ESplineCoordinateSpace::World);
                    DrawDebugSphere(World, FollowingApexPoint + FVector(0, 0, 150), 40.0f, 8, FColor::Purple, false, 0.0f);
                }
            }
        }

        float CurrentSpeed = Vehicle->GetSpeedKmh();
        float CurrentTargetSpeed = CalculateTargetSpeed();
        float MaxCurvature = FindMaxCurvatureAhead();
        float RawOffset = CalculateRacingLineOffset(CurrentSplineDistance + GetLookaheadDistance());


        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f / %.1f km/h"), CurrentSpeed, CurrentTargetSpeed));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,
            FString::Printf(TEXT("Lookahead: %.0f cm"), GetLookaheadDistance()));
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::White,
            FString::Printf(TEXT("Max Curvature: %.3f"), MaxCurvature));
        GEngine->AddOnScreenDebugMessage(4, 0.0f, FColor::White,
            FString::Printf(TEXT("Offset: %.1f (raw: %.1f)"), SmoothedRacingLineOffset, RawOffset));
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

UTurboAction_FollowPath::FCornerInfo UTurboAction_FollowPath::FindNextCorner(float StartDistance, float SearchRange) const
{
    FCornerInfo Result;

    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return Result;
    }

    float SplineLength = Spline->GetSplineLength();
    float MaxCurvature = 0.0f;

    for (float Dist = StartDistance; Dist < StartDistance + SearchRange; Dist += 100.0f)
    {
        float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);

        if (Curvature > MaxCurvature && Curvature > RacingLineMinCurvature)
        {
            MaxCurvature = Curvature;
            Result.ApexDistance = WrappedDist;
            Result.Curvature = Curvature;
            Result.bIsValid = true;
        }
    }

    if (Result.bIsValid)
    {
        Result.TurnSign = RacingSplineActor->GetTurnSign(Result.ApexDistance);
    }

    return Result;
}

UTurboAction_FollowPath::FCornerInfo UTurboAction_FollowPath::FindCornerAfterStraight(float StartDistance, float MaxSearchRange) const
{
    FCornerInfo Result;

    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return Result;
    }

    float SplineLength = Spline->GetSplineLength();

    // First, find where the current corner ends (curvature drops below threshold)
    float StraightStartDistance = StartDistance;
    for (float Dist = StartDistance; Dist < StartDistance + MaxSearchRange; Dist += 100.0f)
    {
        float WrappedDist = Spline->IsClosedLoop() ? FMath::Fmod(Dist, SplineLength) : FMath::Min(Dist, SplineLength);
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(WrappedDist, CurvatureSampleRange);

        if (Curvature < RacingLineMinCurvature)
        {
            StraightStartDistance = WrappedDist;
            break;
        }
    }

    // Now find the next corner after this straight
    float RemainingRange = MaxSearchRange - (StraightStartDistance - StartDistance);
    if (RemainingRange > 0.0f)
    {
        Result = FindNextCorner(StraightStartDistance, RemainingRange);
    }

    return Result;
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

    FCornerInfo NextCorner = FindNextCorner(CurrentSplineDistance, RacingLineLookahead * 2.0f);

    if (!NextCorner.bIsValid)
    {
        return 0.0f;
    }

    float DistanceToApex = NextCorner.ApexDistance - CurrentSplineDistance;
    if (DistanceToApex < 0.0f)
    {
        DistanceToApex += SplineLength;
    }

    float OffsetMagnitude = FMath::Min(NextCorner.Curvature * MaxRacingLineOffset * TrackWidthUsage, MaxRacingLineOffset);

    float ApproachZoneStart = RacingLineLookahead;
    float ApexZoneStart = RacingLineLookahead * 0.25f;

    float Offset = 0.0f;
    FString Zone = TEXT("Unknown");

    if (DistanceToApex > ApproachZoneStart)
    {
        Zone = TEXT("FAR - Outside");
        Offset = -NextCorner.TurnSign * OffsetMagnitude;
    }
    else if (DistanceToApex > ApexZoneStart)
    {
        Zone = TEXT("APPROACH - Transitioning");
        float TransitionProgress = 1.0f - ((DistanceToApex - ApexZoneStart) / (ApproachZoneStart - ApexZoneStart));
        float SmoothProgress = FMath::Sin(TransitionProgress * PI * 0.5f);

        float OutsideOffset = -NextCorner.TurnSign * OffsetMagnitude;
        float InsideOffset = NextCorner.TurnSign * OffsetMagnitude;

        Offset = FMath::Lerp(OutsideOffset, InsideOffset, SmoothProgress);
    }
    else
    {
        Zone = TEXT("APEX - Inside");
        FCornerInfo FollowingCorner = FindCornerAfterStraight(NextCorner.ApexDistance, RacingLineLookahead);

        if (FollowingCorner.bIsValid)
        {
            float PastApexDistance = ApexZoneStart - DistanceToApex;
            float ExitProgress = FMath::Clamp(PastApexDistance / ApexZoneStart, 0.0f, 1.0f);
            float SmoothExit = FMath::Sin(ExitProgress * PI * 0.5f);

            float InsideOffset = NextCorner.TurnSign * OffsetMagnitude;
            float NextOffsetMagnitude = FMath::Min(FollowingCorner.Curvature * MaxRacingLineOffset * TrackWidthUsage, MaxRacingLineOffset);
            float NextOutsideOffset = -FollowingCorner.TurnSign * NextOffsetMagnitude;

            Offset = FMath::Lerp(InsideOffset, NextOutsideOffset, SmoothExit);
        }
        else
        {
            float PastApexDistance = ApexZoneStart - DistanceToApex;
            float ExitProgress = FMath::Clamp(PastApexDistance / ApexZoneStart, 0.0f, 1.0f);

            float InsideOffset = NextCorner.TurnSign * OffsetMagnitude;
            Offset = FMath::Lerp(InsideOffset, 0.0f, ExitProgress * 0.5f);
        }
    }

    return Offset;
}

FVector UTurboAction_FollowPath::GetTargetPoint(float DeltaTime)
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

    FVector CenterlinePoint = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);

    float RawOffset = CalculateRacingLineOffset(TargetDistance);

    if (bUseRacingLineOffset && DeltaTime > 0.0f)
    {
        SmoothedRacingLineOffset = FMath::FInterpTo(
            SmoothedRacingLineOffset,
            RawOffset,
            DeltaTime,
            RacingLineSmoothing
        );

        float MaxChange = MaxOffsetChangeRate * DeltaTime;
        float OffsetDelta = SmoothedRacingLineOffset - PreviousTargetOffset;
        if (FMath::Abs(OffsetDelta) > MaxChange)
        {
            SmoothedRacingLineOffset = PreviousTargetOffset + FMath::Sign(OffsetDelta) * MaxChange;
        }

        PreviousTargetOffset = SmoothedRacingLineOffset;
    }
    else
    {
        SmoothedRacingLineOffset = RawOffset;
        PreviousTargetOffset = RawOffset;
    }

    if (FMath::Abs(SmoothedRacingLineOffset) < 1.0f)
    {
        return CenterlinePoint;
    }

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    return CenterlinePoint + (Right * -SmoothedRacingLineOffset);
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

    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return MaxSpeedKmh;
    }

    float SplineLength = Spline->GetSplineLength();

    FCornerInfo NextCorner = FindNextCorner(CurrentSplineDistance, CornerDetectionDistance);

    if (!NextCorner.bIsValid)
    {
        return MaxSpeedKmh;
    }

    float DistanceToApex = NextCorner.ApexDistance - CurrentSplineDistance;
    if (DistanceToApex < 0.0f)
    {
        DistanceToApex += SplineLength;
    }

    // Calculate required apex speed based on curvature
    float ApexTargetSpeed = MaxSpeedKmh - (NextCorner.Curvature * CurvatureBrakingSensitivity);
    ApexTargetSpeed = FMath::Clamp(ApexTargetSpeed, MinCornerSpeedKmh, MaxSpeedKmh);

    float CurrentSpeed = Vehicle.IsValid() ? Vehicle->GetSpeedKmh() : 0.0f;

    // Very small apex zone - commit only when very close
    float ApexZone = 800.0f;
    if (DistanceToApex < ApexZone)
    {
        return ApexTargetSpeed;
    }

    // Calculate braking distance needed
    float BrakingDecel = 1000.0f * BrakingAggression;
    float CurrentSpeedCms = CurrentSpeed / 0.036f;
    float ApexSpeedCms = ApexTargetSpeed / 0.036f;

    if (CurrentSpeedCms > ApexSpeedCms)
    {
        float BrakingDistanceNeeded = (CurrentSpeedCms * CurrentSpeedCms - ApexSpeedCms * ApexSpeedCms) / (2.0f * BrakingDecel);

        // Minimal safety margin - live on the edge
        float SharpCornerMultiplier = 1.0f + (NextCorner.Curvature * 0.2f);
        BrakingDistanceNeeded *= SharpCornerMultiplier;

        UE_LOG(LogTemp, Warning, TEXT("Curv=%.2f | Apex=%.0f | Speed=%.0f | BrakeDist=%.0f | Dist=%.0f | %s"),
            NextCorner.Curvature,
            ApexTargetSpeed,
            CurrentSpeed,
            BrakingDistanceNeeded,
            DistanceToApex,
            DistanceToApex > BrakingDistanceNeeded ? TEXT("HOLD") : TEXT("BRAKE"));

        if (DistanceToApex <= BrakingDistanceNeeded)
        {
            return ApexTargetSpeed;
        }
    }

    return MaxSpeedKmh;
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

    // Check if we're past the apex (for earlier throttle application)
    USplineComponent* Spline = GetSpline();
    bool bPastApex = false;

    if (Spline && RacingSplineActor.IsValid())
    {
        FCornerInfo NextCorner = FindNextCorner(CurrentSplineDistance, RacingLineLookahead);
        if (NextCorner.bIsValid)
        {
            float DistanceToApex = NextCorner.ApexDistance - CurrentSplineDistance;
            if (DistanceToApex < 0.0f)
            {
                DistanceToApex += Spline->GetSplineLength();
            }

            // Consider "past apex" when within 25% of the approach zone
            bPastApex = DistanceToApex < (RacingLineLookahead * 0.25f);
        }
    }

    if (SpeedError > 0.0f)
    {
        // Need to accelerate
        float ThrottleMultiplier = bPastApex ? ThrottleAggression : 1.0f;
        float ThrottleInput = FMath::Clamp((SpeedError / 20.0f) * ThrottleMultiplier, 0.0f, 1.0f);

        Vehicle->SetThrottleInput(ThrottleInput);
        Vehicle->SetBrakeInput(0.0f);
    }
    else
    {
        // Need to brake
        float BrakeInput = FMath::Clamp((-SpeedError / 30.0f) * BrakingAggression, 0.0f, MaxBrakingForce);

        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(BrakeInput);
    }

    Vehicle->SetHandbrakeInput(false);
}