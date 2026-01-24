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

    // Find the next corner
    FCornerInfo NextCorner = FindNextCorner(CurrentSplineDistance, RacingLineLookahead);

    // No corner ahead - check further for positioning
    if (!NextCorner.bIsValid)
    {
        // Look even further ahead for a distant corner to position for
        FCornerInfo DistantCorner = FindNextCorner(CurrentSplineDistance, RacingLineLookahead * 2.0f);
        if (DistantCorner.bIsValid)
        {
            // Gradually position toward the outside for the distant corner entry
            float DistanceToDistant = DistantCorner.ApexDistance - CurrentSplineDistance;
            if (DistanceToDistant < 0.0f)
            {
                DistanceToDistant += SplineLength;
            }

            // Smooth blend - start positioning when within 2x lookahead
            float BlendFactor = 1.0f - FMath::Clamp(DistanceToDistant / (RacingLineLookahead * 2.0f), 0.0f, 1.0f);
            float OffsetMagnitude = DistantCorner.Curvature * MaxRacingLineOffset * TrackWidthUsage * 0.5f;
            OffsetMagnitude = FMath::Min(OffsetMagnitude, MaxRacingLineOffset);

            // Position on outside for entry (positive phase = outside)
            return -DistantCorner.TurnSign * BlendFactor * OffsetMagnitude;
        }

        return 0.0f;
    }

    // Calculate distance to this corner's apex
    float DistanceToApex = NextCorner.ApexDistance - CurrentSplineDistance;
    if (DistanceToApex < 0.0f)
    {
        DistanceToApex += SplineLength;
    }

    // Check if there's another corner coming after this one
    FCornerInfo FollowingCorner = FindCornerAfterStraight(NextCorner.ApexDistance, RacingLineLookahead * 1.5f);

    // Calculate distance between corners (the "straight" length)
    float StraightLength = 0.0f;
    if (FollowingCorner.bIsValid)
    {
        StraightLength = FollowingCorner.ApexDistance - NextCorner.ApexDistance;
        if (StraightLength < 0.0f)
        {
            StraightLength += SplineLength;
        }
    }

    // Normalize distance to apex (1 = far, 0 = at apex)
    float NormalizedDist = FMath::Clamp(DistanceToApex / RacingLineLookahead, 0.0f, 1.0f);

    // Base phase calculation: cos curve from outside (1) to inside (-1)
    float Phase = FMath::Cos(NormalizedDist * PI);

    // Scale offset by curvature and track width usage
    float OffsetMagnitude = NextCorner.Curvature * MaxRacingLineOffset * TrackWidthUsage;
    OffsetMagnitude = FMath::Min(OffsetMagnitude, MaxRacingLineOffset);

    // Handle corner exit behavior based on what's coming next
    if (FollowingCorner.bIsValid&& NormalizedDist < 0.4f)
    {
        bool bSameDirection = (NextCorner.TurnSign * FollowingCorner.TurnSign) > 0.0f;
        bool bShortStraight = StraightLength < MinStraightForCenterline;

        // How far through the exit are we (0 = at apex, 1 = fully exited)
        float ExitProgress = 1.0f - (NormalizedDist / 0.4f);

        if (bSameDirection)
        {
            // Same direction turn coming - stay on the inside throughout
            // Don't track out, just hold the inside line
            float HoldInsidePhase = -0.7f; // Stay mostly inside
            Phase = FMath::Lerp(Phase, HoldInsidePhase, ExitProgress * 0.8f);
        }
        else if (bShortStraight)
        {
            // Chicane - opposite direction, short straight
            // Blend toward outside for next corner entry
            float NextEntryPhase = 0.8f; // Outside position for next corner
            Phase = FMath::Lerp(Phase, NextEntryPhase, ExitProgress);
        }
        else
        {
            // Long straight before opposite direction corner
            // Smooth transition toward center, then position for next corner
            float TransitionPhase = FMath::Lerp(0.0f, 0.5f * -FollowingCorner.TurnSign / NextCorner.TurnSign, ExitProgress);
            Phase = FMath::Lerp(Phase, TransitionPhase, ExitProgress * 0.6f);
        }
    }
    else if (!FollowingCorner.bIsValid && NormalizedDist < 0.3f)
    {
        // No corner detected after this one - smooth return to center on exit
        float ExitProgress = 1.0f - (NormalizedDist / 0.3f);
        Phase = FMath::Lerp(Phase, 0.0f, ExitProgress * 0.5f);
    }

    return -NextCorner.TurnSign * Phase * OffsetMagnitude;
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

    FVector CenterlinePoint = Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);

    float Offset = CalculateRacingLineOffset(TargetDistance);

    if (FMath::Abs(Offset) < 1.0f)
    {
        return CenterlinePoint;
    }

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