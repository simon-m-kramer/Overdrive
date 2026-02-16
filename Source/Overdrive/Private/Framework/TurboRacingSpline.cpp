// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

ATurboRacingSpline::ATurboRacingSpline()
{
    PrimaryActorTick.bCanEverTick = false;

    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("RacingSpline"));
    SetRootComponent(Spline);

    GameplayTags.AddTag(TurboGameplayTags::Track_MainSpline);
}

void ATurboRacingSpline::BeginPlay()
{
    Super::BeginPlay();

    CalculateRacingLine();
}

// =============================================================================
// RACING LINE CALCULATION
// =============================================================================

void ATurboRacingSpline::CalculateRacingLine()
{
    if (!Spline) return;

    const float SplineLength = Spline->GetSplineLength();

    // ---- Pass 1: Find max curvature for normalization ----
    MaxTrackCurvature = 0.0f;

    for (float Dist = 0.0f; Dist < SplineLength; Dist += RacingLineSampleInterval)
    {
        const float Curvature = GetCurvatureAtDistance(Dist, CurvatureSampleRange);
        MaxTrackCurvature = FMath::Max(MaxTrackCurvature, Curvature);
    }

    if (MaxTrackCurvature < KINDA_SMALL_NUMBER)
    {
        PreCalculatedOffsets.Init(0.0f, FMath::CeilToInt(SplineLength / RacingLineSampleInterval));
        bRacingLineCalculated = true;
        return;
    }

    // ---- Pass 2: Calculate offsets ----
    PreCalculatedOffsets.Empty();

    for (float Dist = 0.0f; Dist < SplineLength; Dist += RacingLineSampleInterval)
    {
        const float Offset = CalculateIdealOffset(Dist);
        PreCalculatedOffsets.Add(Offset);
    }

    bRacingLineCalculated = true;
}

float ATurboRacingSpline::CalculateIdealOffset(float Distance) const
{
    if (!Spline) return 0.0f;

    const float MaxOffset = (TrackWidth * 0.5f) * TrackWidthUsage;

    auto GetNormalizedCurvature = [&](float Dist) -> float
        {
            return GetCurvatureAtDistance(WrapDistance(Dist), CurvatureSampleRange)
                / MaxTrackCurvature;
        };

    const float CurvatureHere = GetNormalizedCurvature(Distance);

    // ----- In a curve: cut toward the inside -----
    if (CurvatureHere >= MinCurvatureThreshold)
    {
        const float TurnDir = GetTurnSign(Distance, TurnSignLookahead);
        if (FMath::Abs(TurnDir) > 0.0f)
        {
            return TurnDir * MaxOffset;
        }
    }

    // ----- On a straight: position fully to the outside of the next curve -----
    for (float Look = LookaheadStepSize; Look < RacingLineLookahead;
        Look += LookaheadStepSize)
    {
        const float LookCurvature = GetNormalizedCurvature(Distance + Look);

        if (LookCurvature <= MinCurvatureThreshold)
            continue;

        // Found a curve — scan forward to find the peak
        float PeakCurvature = 0.0f;
        float PeakDist = Distance + Look;

        for (float Scan = Look; Scan < RacingLineLookahead;
            Scan += LookaheadStepSize)
        {
            const float ScanCurvature = GetNormalizedCurvature(Distance + Scan);

            if (ScanCurvature <= MinCurvatureThreshold)
                break;

            if (ScanCurvature > PeakCurvature)
            {
                PeakCurvature = ScanCurvature;
                PeakDist = Distance + Scan;
            }
        }

        const float TurnDir = GetTurnSign(
            WrapDistance(PeakDist), TurnSignLookahead);

        if (FMath::Abs(TurnDir) > 0.0f)
        {
            return -TurnDir * MaxOffset;
        }

        break;
    }

    return 0.0f;
}

// =============================================================================
// RACING LINE — GETTERS
// =============================================================================

float ATurboRacingSpline::GetRacingLineOffset(float Distance) const
{
    if (!bRacingLineCalculated || PreCalculatedOffsets.Num() == 0 || !Spline)
    {
        return 0.0f;
    }

    Distance = WrapDistance(Distance);

    float IndexFloat = Distance / RacingLineSampleInterval;
    int32 Index = FMath::FloorToInt(IndexFloat);
    float Alpha = IndexFloat - Index;

    Index = FMath::Clamp(Index, 0, PreCalculatedOffsets.Num() - 1);
    int32 NextIndex = (Index + 1) % PreCalculatedOffsets.Num();

    return FMath::Lerp(PreCalculatedOffsets[Index], PreCalculatedOffsets[NextIndex], Alpha);
}

FVector ATurboRacingSpline::GetPointOnRacingLine(float Distance) const
{
    if (!Spline)
    {
        return FVector::ZeroVector;
    }

    Distance = WrapDistance(Distance);

    FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

    float Offset = GetRacingLineOffset(Distance);
    if (FMath::Abs(Offset) < 1.0f)
    {
        return CenterPoint;
    }

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Up, Tangent).GetSafeNormal();

    return CenterPoint + (Right * Offset);
}

// =============================================================================
// CURVATURE ANALYSIS
// =============================================================================

float ATurboRacingSpline::WrapDistance(float Distance) const
{
    if (!Spline) return Distance;
    const float SplineLength = Spline->GetSplineLength();
    if (SplineLength <= KINDA_SMALL_NUMBER) return 0.0f;  // handling zero-length spline

    if (Spline->IsClosedLoop())
    {
        Distance = FMath::Fmod(Distance, SplineLength);
        if (Distance < 0.0f) Distance += SplineLength;  // negative FMod correction
    }
    else
    {
        Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);  // making sure that the end of the track is handled gracefully
    }

    return Distance;
}

float ATurboRacingSpline::GetCurvatureAtDistance(float Distance, float SampleRange) const
{
    if (!Spline) return 0.0f;

    const float HalfRange = SampleRange * 0.5f;

    const float DistA = WrapDistance(Distance - HalfRange);
    const float DistB = WrapDistance(Distance + HalfRange);

    const FVector TangentA = Spline->GetDirectionAtDistanceAlongSpline(DistA, ESplineCoordinateSpace::World);
    const FVector TangentB = Spline->GetDirectionAtDistanceAlongSpline(DistB, ESplineCoordinateSpace::World);

    const float Dot = FVector::DotProduct(TangentA, TangentB);
    const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));  // clamp is a floating point error safeguard;

    return Angle / SampleRange;
}

float ATurboRacingSpline::GetTurnSign(float Distance, float InLookaheadDistance) const
{
    if (!Spline) return 0.0f;

    const float HalfRange = InLookaheadDistance * 0.5f;

    FVector TangentA = Spline->GetDirectionAtDistanceAlongSpline(WrapDistance(Distance - HalfRange), ESplineCoordinateSpace::World);
    FVector TangentB = Spline->GetDirectionAtDistanceAlongSpline(WrapDistance(Distance + HalfRange), ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(WrapDistance(Distance), ESplineCoordinateSpace::World);

    FVector Cross = FVector::CrossProduct(TangentA, TangentB);
    float DotUp = FVector::DotProduct(Cross, Up);

    if (FMath::Abs(DotUp) < KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    return (DotUp > 0.0f) ? 1.0f : -1.0f;
}

// =============================================================================
// DEBUG DRAWING
// =============================================================================

void ATurboRacingSpline::DrawDebugRacingLine(UWorld* World) const
{
    if (!bRacingLineCalculated || !Spline || !World)
    {
        return;
    }

    float SplineLength = Spline->GetSplineLength();

    FVector PreviousPoint = FVector::ZeroVector;
    bool bFirstPoint = true;

    for (float Dist = 0.0f; Dist < SplineLength; Dist += RacingLineSampleInterval)
    {
        FVector RacingLinePoint = GetPointOnRacingLine(Dist) + FVector(0.0f, 0.0f, 20.0f);
        float Offset = GetRacingLineOffset(Dist);

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
        FVector FirstPoint = GetPointOnRacingLine(0.0f) + FVector(0.0f, 0.0f, 20.0f);
        DrawDebugLine(World, PreviousPoint, FirstPoint, FColor::Yellow, false, 0.0f, 0, 2.0f);
    }
}