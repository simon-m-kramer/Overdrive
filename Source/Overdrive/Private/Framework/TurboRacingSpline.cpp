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
        // Track is essentially straight — no offsets needed
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

    // ---- Pass 3: Smoothing ----

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
                const int32 Index = (i + j + PreCalculatedOffsets.Num()) % PreCalculatedOffsets.Num();
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

float ATurboRacingSpline::CalculateIdealOffset(float Distance) const
{
    if (!Spline) return 0.0f;

    const float MaxOffset = (TrackWidth * 0.5f) * TrackWidthUsage;

    // Normalized curvature: 0 = straight, 1 = tightest corner on track
    auto GetNormalizedCurvature = [&](float Dist) -> float
        {
            return GetCurvatureAtDistance(WrapDistance(Dist), CurvatureSampleRange) / MaxTrackCurvature;
        };

    const float CurvatureHere = GetNormalizedCurvature(Distance);
    const float CurvatureAhead = GetNormalizedCurvature(Distance + ApproachSampleDistance);

    // ---- On a straight: look ahead for the next turn, position to outside ----

    if (CurvatureHere < MinCurvatureThreshold && CurvatureAhead < MinCurvatureThreshold)
    {
        for (float Look = ApproachSampleDistance; Look < RacingLineLookahead; Look += LookaheadStepSize)
        {
            const float LookDist = WrapDistance(Distance + Look);
            const float LookCurvature = GetNormalizedCurvature(Distance + Look);

            if (LookCurvature > MinCurvatureThreshold)
            {
                const float TurnDir = GetTurnSign(LookDist, TurnSignLookahead);
                return -TurnDir * MaxOffset; // outside of upcoming turn, full width
            }
        }

        return 0.0f; // no turn ahead, stay centered
    }

    // ---- In a curve: full offset to the inside ----

    const float TurnDir = GetTurnSign(Distance, TurnSignLookahead);
    return TurnDir * MaxOffset;
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
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    return CenterPoint + (Right * -Offset);
}

// =============================================================================
// UTILITY
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

// =============================================================================
// CURVATURE ANALYSIS
// =============================================================================

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

    return Angle / SampleRange;  // this is the derivative in radians/cm
}

float ATurboRacingSpline::GetTurnSign(float Distance, float InLookaheadDistance) const
{
    if (!Spline) return 0.0f;

    float CurrentWrappedDist = WrapDistance(Distance);
    FVector CurrentPos = Spline->GetLocationAtDistanceAlongSpline(CurrentWrappedDist, ESplineCoordinateSpace::World);
    FVector RightVec = Spline->GetRightVectorAtDistanceAlongSpline(CurrentWrappedDist, ESplineCoordinateSpace::World);

    float FutureWrappedDist = WrapDistance(Distance + InLookaheadDistance);
    FVector FuturePos = Spline->GetLocationAtDistanceAlongSpline(FutureWrappedDist, ESplineCoordinateSpace::World);

    FVector Delta = (FuturePos - CurrentPos).GetSafeNormal();

    float DotResult = FVector::DotProduct(Delta, RightVec);

    if (FMath::Abs(DotResult) < 0.01f)
    {
        return 0.0f;
    }

    return (DotResult > 0.0f) ? 1.0f : -1.0f;
}