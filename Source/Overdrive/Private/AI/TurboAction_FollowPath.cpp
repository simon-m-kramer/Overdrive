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

        SmoothedRacingLineOffset = 0.0f;
        PreviousTargetOffset = 0.0f;

        // Pre-calculate the racing line
        if (bUseRacingLineOffset)
        {
            PreCalculateRacingLine();
        }
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
        // Existing debug drawing...
        UWorld* World = Vehicle->GetWorld();
        FVector VehicleLocation = Vehicle->GetActorLocation();
        USplineComponent* Spline = GetSpline();

        DrawDebugSphere(World, TargetPoint, 50.0f, 12, FColor::Green, false, 0.0f);
        DrawDebugLine(World, VehicleLocation, TargetPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

        FVector ForwardEnd = VehicleLocation + Vehicle->GetActorForwardVector() * 500.0f;
        DrawDebugLine(World, VehicleLocation, ForwardEnd, FColor::Red, false, 0.0f, 0, 3.0f);

        FVector CurrentSplinePoint = Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
        DrawDebugSphere(World, CurrentSplinePoint, 30.0f, 8, FColor::Blue, false, 0.0f);

        // Draw the full racing line
        DrawDebugRacingLine();

        float CurrentSpeed = Vehicle->GetSpeedKmh();
        float CurrentTargetSpeed = CalculateTargetSpeed();

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f / %.1f km/h"), CurrentSpeed, CurrentTargetSpeed));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,
            FString::Printf(TEXT("Lookahead: %.0f cm"), GetLookaheadDistance()));
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
    if (!bUseRacingLineOffset || !bRacingLineCalculated)
    {
        return 0.0f;
    }

    return GetPreCalculatedOffset(AtDistance);
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
        // Check if we're in a critical transition (big offset change needed)
        float OffsetDifference = FMath::Abs(RawOffset - SmoothedRacingLineOffset);
        bool bCriticalTransition = OffsetDifference > MaxRacingLineOffset * 0.5f;

        if (bCriticalTransition)
        {
            // Aggressive tracking - minimal smoothing
            SmoothedRacingLineOffset = FMath::FInterpTo(
                SmoothedRacingLineOffset,
                RawOffset,
                DeltaTime,
                RacingLineSmoothing * 3.0f  // Triple the smoothing speed
            );

            // Higher rate limit during critical transition
            float MaxChange = MaxOffsetChangeRate * 3.0f * DeltaTime;
            float OffsetDelta = SmoothedRacingLineOffset - PreviousTargetOffset;
            if (FMath::Abs(OffsetDelta) > MaxChange)
            {
                SmoothedRacingLineOffset = PreviousTargetOffset + FMath::Sign(OffsetDelta) * MaxChange;
            }
        }
        else
        {
            // Normal smoothing
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

    // Base steering
    float SteeringInput = DotRight * SteeringMultiplier;

    // Apply aggressive steering when approaching sharp corners at speed
    float MaxCurvature = FindMaxCurvatureAhead();
    float CurrentSpeed = Vehicle->GetSpeedKmh();

    if (MaxCurvature > AggressiveSteeringCurvatureThreshold && CurrentSpeed > MinCornerSpeedKmh)
    {
        // Scale aggression by how sharp the corner is and how fast we're going
        float CurvatureFactor = (MaxCurvature - AggressiveSteeringCurvatureThreshold) / (1.0f - AggressiveSteeringCurvatureThreshold);
        float SpeedFactor = FMath::Clamp((CurrentSpeed - MinCornerSpeedKmh) / (MaxSpeedKmh - MinCornerSpeedKmh), 0.0f, 1.0f);

        float AggressionBoost = 1.0f + (AggressiveSteeringMultiplier - 1.0f) * CurvatureFactor * SpeedFactor;
        SteeringInput *= AggressionBoost;
    }

    return FMath::Clamp(SteeringInput, -1.0f, 1.0f);
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

    // Dynamic minimum speed based on curvature
    // Very sharp corners (hairpins) need much slower speeds
    float DynamicMinSpeed = MinCornerSpeedKmh;
    if (MaxCurvature > 0.8f)
    {
        // Interpolate from MinCornerSpeedKmh to HairpinSpeedKmh for curvature 0.8 to 1.0
        float HairpinFactor = (MaxCurvature - 0.8f) / 0.2f;
        DynamicMinSpeed = FMath::Lerp(MinCornerSpeedKmh, HairpinSpeedKmh, HairpinFactor);
    }

    float SpeedReduction = MaxCurvature * CurvatureBrakingSensitivity;
    float DesiredSpeed = MaxSpeedKmh - SpeedReduction;

    return FMath::Clamp(DesiredSpeed, DynamicMinSpeed, MaxSpeedKmh);
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

    if (SpeedError > ThrottleDeadzone)
    {
        // Need to accelerate significantly
        float ThrottleInput = FMath::Clamp((SpeedError - ThrottleDeadzone) / 20.0f, 0.0f, 1.0f);
        Vehicle->SetThrottleInput(ThrottleInput);
        Vehicle->SetBrakeInput(0.0f);
    }
    else if (SpeedError < -CoastingThreshold)
    {
        // Need to brake significantly
        float BrakeInput = FMath::Clamp((-SpeedError - CoastingThreshold) / 30.0f, 0.0f, 1.0f);
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(BrakeInput);
    }
    else
    {
        // Coast - let friction and engine braking do the work
        Vehicle->SetThrottleInput(0.0f);
        Vehicle->SetBrakeInput(0.0f);
    }

    Vehicle->SetHandbrakeInput(false);
}

void UTurboAction_FollowPath::PreCalculateRacingLine()
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return;
    }

    PreCalculatedOffsets.Empty();
    float SplineLength = Spline->GetSplineLength();

    // First pass: Calculate raw offsets
    for (float Dist = 0.0f; Dist < SplineLength; Dist += RacingLineSampleInterval)
    {
        float Offset = CalculateIdealOffset(Dist);
        PreCalculatedOffsets.Add(Offset);
    }

    // Multiple smoothing passes for a cleaner line
    int32 NumSmoothingPasses = 3;
    int32 SmoothingWindow = 10;  // Average over 10 samples (1000cm / 10m)

    for (int32 Pass = 0; Pass < NumSmoothingPasses; Pass++)
    {
        TArray<float> SmoothedOffsets;

        for (int32 i = 0; i < PreCalculatedOffsets.Num(); i++)
        {
            float Sum = 0.0f;
            float WeightSum = 0.0f;

            for (int32 j = -SmoothingWindow; j <= SmoothingWindow; j++)
            {
                int32 Index = (i + j + PreCalculatedOffsets.Num()) % PreCalculatedOffsets.Num();

                // Gaussian-like weight: center samples have more influence
                float Weight = 1.0f - (FMath::Abs(j) / (float)(SmoothingWindow + 1));
                Weight = Weight * Weight;  // Square for smoother falloff

                Sum += PreCalculatedOffsets[Index] * Weight;
                WeightSum += Weight;
            }

            SmoothedOffsets.Add(Sum / WeightSum);
        }

        PreCalculatedOffsets = SmoothedOffsets;
    }

    bRacingLineCalculated = true;

    UE_LOG(LogTemp, Warning, TEXT("Racing line calculated: %d points over %.0f cm"),
        PreCalculatedOffsets.Num(), SplineLength);
}

float UTurboAction_FollowPath::CalculateIdealOffset(float Distance) const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return 0.0f;
    }

    float SplineLength = Spline->GetSplineLength();
    float SampleDistance = 800.0f;

    // Wrap distances for closed loop
    auto WrapDistance = [SplineLength, Spline](float Dist) -> float
        {
            if (Spline->IsClosedLoop())
            {
                while (Dist < 0.0f) Dist += SplineLength;
                while (Dist >= SplineLength) Dist -= SplineLength;
            }
            else
            {
                Dist = FMath::Clamp(Dist, 0.0f, SplineLength);
            }
            return Dist;
        };

    float DistBehind = WrapDistance(Distance - SampleDistance);
    float DistAhead = WrapDistance(Distance + SampleDistance);

    float CurvatureBehind = RacingSplineActor->GetCurvatureAtDistance(DistBehind, CurvatureSampleRange);
    float CurvatureCurrent = RacingSplineActor->GetCurvatureAtDistance(Distance, CurvatureSampleRange);
    float CurvatureAhead = RacingSplineActor->GetCurvatureAtDistance(DistAhead, CurvatureSampleRange);

    // Not in or near a corner - stay on centerline
    if (CurvatureCurrent < RacingLineMinCurvature && CurvatureAhead < RacingLineMinCurvature)
    {
        // Look further ahead for upcoming corner to position for outside
        for (float LookAhead = SampleDistance; LookAhead < RacingLineLookahead; LookAhead += 200.0f)
        {
            float LookDist = WrapDistance(Distance + LookAhead);
            float LookCurvature = RacingSplineActor->GetCurvatureAtDistance(LookDist, CurvatureSampleRange);

            if (LookCurvature > RacingLineMinCurvature)
            {
                // Found upcoming corner - position on outside
                float TurnSign = RacingSplineActor->GetTurnSign(LookDist);
                float OffsetMagnitude = FMath::Min(LookCurvature * MaxRacingLineOffset * TrackWidthUsage, MaxRacingLineOffset);
                return -TurnSign * OffsetMagnitude;
            }
        }
        return 0.0f;
    }

    float TurnSign = RacingSplineActor->GetTurnSign(Distance);
    float OffsetMagnitude = FMath::Min(CurvatureCurrent * MaxRacingLineOffset * TrackWidthUsage, MaxRacingLineOffset);

    // Determine corner phase based on curvature gradient
    bool bCurvatureIncreasing = CurvatureAhead > CurvatureCurrent + 0.02f;
    bool bCurvatureDecreasing = CurvatureBehind > CurvatureCurrent + 0.02f;
    bool bAtApex = !bCurvatureIncreasing && !bCurvatureDecreasing && CurvatureCurrent > RacingLineMinCurvature;

    float Offset = 0.0f;

    if (bCurvatureIncreasing)
    {
        // ENTRY - Transition from outside to inside
        float ApproachFactor = CurvatureCurrent / FMath::Max(CurvatureAhead, 0.01f);
        ApproachFactor = FMath::Clamp(ApproachFactor, 0.0f, 1.0f);

        // Start outside, move to inside as we approach apex
        float InsideFactor = (ApproachFactor * 2.0f) - 1.0f;  // -1 to +1
        Offset = TurnSign * OffsetMagnitude * InsideFactor;
    }
    else if (bAtApex)
    {
        // APEX - Full inside
        Offset = TurnSign * OffsetMagnitude;
    }
    else if (bCurvatureDecreasing)
    {
        // EXIT - Track out gradually
        float ExitFactor = CurvatureCurrent / FMath::Max(CurvatureBehind, 0.01f);
        ExitFactor = FMath::Clamp(ExitFactor, 0.0f, 1.0f);
        Offset = TurnSign * OffsetMagnitude * ExitFactor;
    }
    else
    {
        // Constant curvature - hold inside
        Offset = TurnSign * OffsetMagnitude;
    }

    return Offset;
}

float UTurboAction_FollowPath::GetPreCalculatedOffset(float Distance) const
{
    if (!bRacingLineCalculated || PreCalculatedOffsets.Num() == 0)
    {
        return 0.0f;
    }

    USplineComponent* Spline = GetSpline();
    float SplineLength = Spline->GetSplineLength();

    // Wrap distance for closed loop
    if (Spline->IsClosedLoop())
    {
        while (Distance < 0.0f) Distance += SplineLength;
        while (Distance >= SplineLength) Distance -= SplineLength;
    }

    float IndexFloat = Distance / RacingLineSampleInterval;
    int32 Index = FMath::FloorToInt(IndexFloat);
    int32 NextIndex = (Index + 1) % PreCalculatedOffsets.Num();
    float Alpha = IndexFloat - Index;

    Index = FMath::Clamp(Index, 0, PreCalculatedOffsets.Num() - 1);

    return FMath::Lerp(PreCalculatedOffsets[Index], PreCalculatedOffsets[NextIndex], Alpha);
}

void UTurboAction_FollowPath::DrawDebugRacingLine() const
{
    if (!bDrawDebug || !bRacingLineCalculated)
    {
        return;
    }

    USplineComponent* Spline = GetSpline();
    if (!Spline || !Vehicle.IsValid())
    {
        return;
    }

    UWorld* World = Vehicle->GetWorld();
    float SplineLength = Spline->GetSplineLength();

    FVector PreviousPoint = FVector::ZeroVector;
    bool bFirstPoint = true;

    for (float Dist = 0.0f; Dist < SplineLength; Dist += RacingLineSampleInterval)
    {
        float Offset = GetPreCalculatedOffset(Dist);

        FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
        FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
        FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
        FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

        FVector RacingLinePoint = CenterPoint + (Right * -Offset) + FVector(0, 0, 20);  // Slight Z offset for visibility

        // Color based on offset: green = inside, red = outside, yellow = center
        FColor PointColor;
        if (FMath::Abs(Offset) < 50.0f)
        {
            PointColor = FColor::Yellow;
        }
        else if (Offset > 0)
        {
            PointColor = FColor::Green;  // Inside (positive offset after sign flip)
        }
        else
        {
            PointColor = FColor::Red;    // Outside (negative offset)
        }

        DrawDebugPoint(World, RacingLinePoint, 8.0f, PointColor, false, 0.0f);

        if (!bFirstPoint)
        {
            DrawDebugLine(World, PreviousPoint, RacingLinePoint, PointColor, false, 0.0f, 0, 2.0f);
        }

        PreviousPoint = RacingLinePoint;
        bFirstPoint = false;
    }

    // Connect last point to first for closed loop
    if (Spline->IsClosedLoop() && !bFirstPoint)
    {
        float Offset = GetPreCalculatedOffset(0.0f);
        FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();
        FVector FirstPoint = CenterPoint + (Right * -Offset) + FVector(0, 0, 20);

        DrawDebugLine(World, PreviousPoint, FirstPoint, FColor::Yellow, false, 0.0f, 0, 2.0f);
    }
}

