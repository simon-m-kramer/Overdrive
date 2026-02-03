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

    FVector TargetPoint = GetTargetPoint();

    if (bDrawDebug)
    {
        UWorld* World = Vehicle->GetWorld();
        FVector VehicleLocation = Vehicle->GetActorLocation();
        USplineComponent* Spline = GetSpline();

        DrawDebugSphere(World, TargetPoint, 50.0f, 12, FColor::Green, false, 0.0f);
        DrawDebugLine(World, VehicleLocation, TargetPoint, FColor::Yellow, false, 0.0f, 0, 3.0f);

        FVector ForwardEnd = VehicleLocation + Vehicle->GetActorForwardVector() * 500.0f;
        DrawDebugLine(World, VehicleLocation, ForwardEnd, FColor::Red, false, 0.0f, 0, 3.0f);

        FVector CurrentSplinePoint = Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
        DrawDebugSphere(World, CurrentSplinePoint, 30.0f, 8, FColor::Blue, false, 0.0f);

        DrawDebugRacingLine();

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            FString::Printf(TEXT("Speed: %.1f km/h"), Vehicle->GetSpeedKmh()));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,
            FString::Printf(TEXT("Lookahead: %.0f cm"), GetLookaheadDistance()));
    }

    float SteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(SteeringInput);

    // TODO: Add speed control
    Vehicle->SetThrottleInput(0.5f);
    Vehicle->SetBrakeInput(0.0f);
    Vehicle->SetHandbrakeInput(false);
}

// =============================================================================
// CORE METHODS
// =============================================================================

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

FVector UTurboAction_FollowPath::GetTargetPoint()
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

    return CenterlinePoint + (Right * -Offset);
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
// RACING LINE
// =============================================================================

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

    // Multiple smoothing passes with triangular kernel
    for (int32 Pass = 0; Pass < SmoothingPasses; Pass++)
    {
        TArray<float> SmoothedOffsets;
        SmoothedOffsets.Reserve(PreCalculatedOffsets.Num());

        for (int32 i = 0; i < PreCalculatedOffsets.Num(); i++)
        {
            float Sum = 0.0f;
            float WeightSum = 0.0f;

            for (int32 j = -SmoothingWindow; j <= SmoothingWindow; j++)
            {
                int32 Index = (i + j + PreCalculatedOffsets.Num()) % PreCalculatedOffsets.Num();

                float Weight = 1.0f - (FMath::Abs(j) / static_cast<float>(SmoothingWindow + 1));
                Weight = Weight * Weight;

                Sum += PreCalculatedOffsets[Index] * Weight;
                WeightSum += Weight;
            }

            SmoothedOffsets.Add(Sum / WeightSum);
        }

        PreCalculatedOffsets = MoveTemp(SmoothedOffsets);
    }

    bRacingLineCalculated = true;
}

float UTurboAction_FollowPath::CalculateIdealOffset(float Distance) const
{
    USplineComponent* Spline = GetSpline();
    if (!Spline || !RacingSplineActor.IsValid())
    {
        return 0.0f;
    }

    float SplineLength = Spline->GetSplineLength();
    bool bIsClosedLoop = Spline->IsClosedLoop();
    float MaxOffset = (TrackWidth * 0.5f) * TrackWidthUsage;

    auto WrapDistance = [SplineLength, bIsClosedLoop](float Dist) -> float
        {
            if (bIsClosedLoop)
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

    float DistBehind = WrapDistance(Distance - ApproachSampleDistance);
    float DistAhead = WrapDistance(Distance + ApproachSampleDistance);

    // Use true curvature (radians/distance)
    float CurvatureBehind = RacingSplineActor->GetCurvatureAtDistance(DistBehind, CurvatureSampleRange);
    float CurvatureCurrent = RacingSplineActor->GetCurvatureAtDistance(Distance, CurvatureSampleRange);
    float CurvatureAhead = RacingSplineActor->GetCurvatureAtDistance(DistAhead, CurvatureSampleRange);

    // Not in or near a corner - look ahead for upcoming turns
    if (CurvatureCurrent < RacingLineMinCurvature && CurvatureAhead < RacingLineMinCurvature)
    {
        for (float LookAhead = ApproachSampleDistance; LookAhead < RacingLineLookahead; LookAhead += LookaheadStepSize)
        {
            float LookDist = WrapDistance(Distance + LookAhead);
            float LookCurvature = RacingSplineActor->GetCurvatureAtDistance(LookDist, CurvatureSampleRange);

            if (LookCurvature > RacingLineMinCurvature)
            {
                float TurnSign = RacingSplineActor->GetTurnSign(LookDist, TurnSignLookahead);
                float OffsetMagnitude = FMath::Min(LookCurvature * CurvatureToOffsetScale * TrackWidthUsage, MaxOffset);
                return -TurnSign * OffsetMagnitude;
            }
        }
        return 0.0f;
    }

    float TurnSign = RacingSplineActor->GetTurnSign(Distance, TurnSignLookahead);
    float OffsetMagnitude = FMath::Min(CurvatureCurrent * CurvatureToOffsetScale * TrackWidthUsage, MaxOffset);

    // Use relative threshold for curvature change detection
    float CurvatureChangeThreshold = FMath::Max(CurvatureCurrent * CurvatureChangePercent, RacingLineMinCurvature * 0.5f);

    bool bCurvatureIncreasing = CurvatureAhead > CurvatureCurrent + CurvatureChangeThreshold;
    bool bCurvatureDecreasing = CurvatureBehind > CurvatureCurrent + CurvatureChangeThreshold;
    bool bAtApex = !bCurvatureIncreasing && !bCurvatureDecreasing && CurvatureCurrent > RacingLineMinCurvature;

    float Offset = 0.0f;

    if (bCurvatureIncreasing)
    {
        // CORNER ENTRY: Transition from outside to inside
        float ApproachFactor = CurvatureCurrent / FMath::Max(CurvatureAhead, RacingLineMinCurvature);
        ApproachFactor = FMath::Clamp(ApproachFactor, 0.0f, 1.0f);
        float InsideFactor = (ApproachFactor * 2.0f) - 1.0f;
        Offset = TurnSign * OffsetMagnitude * InsideFactor;
    }
    else if (bAtApex)
    {
        // APEX: Full inside offset
        Offset = TurnSign * OffsetMagnitude;
    }
    else if (bCurvatureDecreasing)
    {
        // CORNER EXIT: Transition from inside back toward center
        float ExitFactor = CurvatureCurrent / FMath::Max(CurvatureBehind, RacingLineMinCurvature);
        ExitFactor = FMath::Clamp(ExitFactor, 0.0f, 1.0f);
        Offset = TurnSign * OffsetMagnitude * ExitFactor;
    }
    else
    {
        // Sustained corner
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
    if (!Spline)
    {
        return 0.0f;
    }

    float SplineLength = Spline->GetSplineLength();

    if (Spline->IsClosedLoop())
    {
        while (Distance < 0.0f) Distance += SplineLength;
        while (Distance >= SplineLength) Distance -= SplineLength;
    }
    else
    {
        Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);
    }

    float IndexFloat = Distance / RacingLineSampleInterval;
    int32 Index = FMath::FloorToInt(IndexFloat);
    float Alpha = IndexFloat - Index;

    Index = FMath::Clamp(Index, 0, PreCalculatedOffsets.Num() - 1);
    int32 NextIndex = (Index + 1) % PreCalculatedOffsets.Num();

    return FMath::Lerp(PreCalculatedOffsets[Index], PreCalculatedOffsets[NextIndex], Alpha);
}

float UTurboAction_FollowPath::CalculateRacingLineOffset(float AtDistance) const
{
    if (!bUseRacingLineOffset || !bRacingLineCalculated)
    {
        return 0.0f;
    }

    return GetPreCalculatedOffset(AtDistance);
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
    if (!World)
    {
        return;
    }

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

        FVector RacingLinePoint = CenterPoint + (Right * -Offset) + FVector(0.0f, 0.0f, 20.0f);

        FColor PointColor;
        if (FMath::Abs(Offset) < 50.0f)
        {
            PointColor = FColor::Yellow;
        }
        else if (Offset > 0.0f)
        {
            PointColor = FColor::Green;
        }
        else
        {
            PointColor = FColor::Red;
        }

        DrawDebugPoint(World, RacingLinePoint, 8.0f, PointColor, false, 0.0f);

        if (!bFirstPoint)
        {
            DrawDebugLine(World, PreviousPoint, RacingLinePoint, PointColor, false, 0.0f, 0, 2.0f);
        }

        PreviousPoint = RacingLinePoint;
        bFirstPoint = false;
    }

    if (Spline->IsClosedLoop() && !bFirstPoint)
    {
        float Offset = GetPreCalculatedOffset(0.0f);
        FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
        FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();
        FVector FirstPoint = CenterPoint + (Right * -Offset) + FVector(0.0f, 0.0f, 20.0f);

        DrawDebugLine(World, PreviousPoint, FirstPoint, FColor::Yellow, false, 0.0f, 0, 2.0f);
    }
}