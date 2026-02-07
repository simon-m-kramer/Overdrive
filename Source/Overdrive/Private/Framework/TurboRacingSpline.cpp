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
// TRACK BOUNDARIES
// =============================================================================

bool ATurboRacingSpline::IsOffTrack(FVector WorldLocation) const
{
    float Distance = GetDistanceFromCenter(WorldLocation);
    float HalfWidth = TrackWidth * 0.5f;

    return Distance > HalfWidth;
}

float ATurboRacingSpline::GetDistanceFromCenter(FVector WorldLocation) const
{
    return FMath::Abs(GetSignedDistanceFromCenter(WorldLocation));
}

float ATurboRacingSpline::GetSignedDistanceFromCenter(FVector WorldLocation) const
{
    if (!Spline)
    {
        return 0.0f;
    }

    // Find closest point on spline
    float SplineDistance = Spline->GetDistanceAlongSplineAtLocation(WorldLocation, ESplineCoordinateSpace::World);
    FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);

    // Get right vector at this point
    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(SplineDistance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    // Project offset onto right vector (positive = right of center, negative = left)
    FVector ToLocation = WorldLocation - CenterPoint;
    float SignedDistance = FVector::DotProduct(ToLocation, Right);

    return SignedDistance;
}

FVector ATurboRacingSpline::GetLeftEdgeAtDistance(float Distance) const
{
    if (!Spline)
    {
        return FVector::ZeroVector;
    }

    Distance = WrapDistance(Distance);

    FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    float HalfWidth = TrackWidth * 0.5f;

    return CenterPoint - (Right * HalfWidth);
}

FVector ATurboRacingSpline::GetRightEdgeAtDistance(float Distance) const
{
    if (!Spline)
    {
        return FVector::ZeroVector;
    }

    Distance = WrapDistance(Distance);

    FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    float HalfWidth = TrackWidth * 0.5f;

    return CenterPoint + (Right * HalfWidth);
}

void ATurboRacingSpline::DrawDebugTrackBoundaries(UWorld* World) const
{
    if (!Spline || !World)
    {
        return;
    }

    float SplineLength = Spline->GetSplineLength();
    float SampleInterval = RacingLineSampleInterval;
    float DebugHeight = 30.0f;

    FVector PrevLeftPoint = FVector::ZeroVector;
    FVector PrevRightPoint = FVector::ZeroVector;
    bool bFirstPoint = true;

    for (float Dist = 0.0f; Dist < SplineLength; Dist += SampleInterval)
    {
        FVector LeftPoint = GetLeftEdgeAtDistance(Dist) + FVector(0.0f, 0.0f, DebugHeight);
        FVector RightPoint = GetRightEdgeAtDistance(Dist) + FVector(0.0f, 0.0f, DebugHeight);

        DrawDebugPoint(World, LeftPoint, 6.0f, FColor::White, false, 0.0f);
        DrawDebugPoint(World, RightPoint, 6.0f, FColor::White, false, 0.0f);

        if (!bFirstPoint)
        {
            DrawDebugLine(World, PrevLeftPoint, LeftPoint, FColor::White, false, 0.0f, 0, 1.5f);
            DrawDebugLine(World, PrevRightPoint, RightPoint, FColor::White, false, 0.0f, 0, 1.5f);
        }

        PrevLeftPoint = LeftPoint;
        PrevRightPoint = RightPoint;
        bFirstPoint = false;
    }

    // Close the loop
    if (Spline->IsClosedLoop() && !bFirstPoint)
    {
        FVector FirstLeftPoint = GetLeftEdgeAtDistance(0.0f) + FVector(0.0f, 0.0f, DebugHeight);
        FVector FirstRightPoint = GetRightEdgeAtDistance(0.0f) + FVector(0.0f, 0.0f, DebugHeight);

        DrawDebugLine(World, PrevLeftPoint, FirstLeftPoint, FColor::White, false, 0.0f, 0, 1.5f);
        DrawDebugLine(World, PrevRightPoint, FirstRightPoint, FColor::White, false, 0.0f, 0, 1.5f);
    }
}

// =============================================================================
// RACING LINE
// =============================================================================

void ATurboRacingSpline::CalculateRacingLine()
{
    if (!Spline)
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

    // Generate overtake lane — opposite side of racing line
    float MaxOffset = (TrackWidth * 0.5f) * OvertakeLaneWidthUsage;

    PreCalculatedOvertakeOffsets.Empty();
    PreCalculatedOvertakeOffsets.Reserve(PreCalculatedOffsets.Num());

    for (int32 i = 0; i < PreCalculatedOffsets.Num(); i++)
    {
        float RacingOffset = PreCalculatedOffsets[i];

        // Mirror but stay closer to center in corners
        float CornerScale = 0.4f;  // tune this — lower = closer to center
        float OvertakeOffset = -RacingOffset * CornerScale;

        // On straights (racing line near center), push overtake lane to one side
        if (FMath::Abs(RacingOffset) < MaxOffset * 0.2f)
        {
            // Look ahead to find which side the next corner's racing line goes
            int32 LookAheadSamples = FMath::CeilToInt(2000.0f / RacingLineSampleInterval);
            float MaxFutureOffset = 0.0f;

            for (int32 j = 1; j <= LookAheadSamples; j++)
            {
                int32 FutureIndex = (i + j) % PreCalculatedOffsets.Num();
                if (FMath::Abs(PreCalculatedOffsets[FutureIndex]) > FMath::Abs(MaxFutureOffset))
                {
                    MaxFutureOffset = PreCalculatedOffsets[FutureIndex];
                }
            }

            // Position on opposite side of where the racing line will go
            if (FMath::Abs(MaxFutureOffset) > MaxOffset * 0.2f)
            {
                OvertakeOffset = -FMath::Sign(MaxFutureOffset) * MaxOffset * 0.7f;
            }
        }

        // Clamp to track boundaries
        OvertakeOffset = FMath::Clamp(OvertakeOffset, -MaxOffset, MaxOffset);

        PreCalculatedOvertakeOffsets.Add(OvertakeOffset);
    }

    // Smooth the overtake lane
    for (int32 Pass = 0; Pass < SmoothingPasses; Pass++)
    {
        TArray<float> Smoothed;
        Smoothed.Reserve(PreCalculatedOvertakeOffsets.Num());

        for (int32 i = 0; i < PreCalculatedOvertakeOffsets.Num(); i++)
        {
            float Sum = 0.0f;
            float WeightSum = 0.0f;

            for (int32 j = -SmoothingWindow; j <= SmoothingWindow; j++)
            {
                int32 Index = (i + j + PreCalculatedOvertakeOffsets.Num()) % PreCalculatedOvertakeOffsets.Num();
                float Weight = 1.0f - (FMath::Abs(j) / static_cast<float>(SmoothingWindow + 1));
                Weight = Weight * Weight;

                Sum += PreCalculatedOvertakeOffsets[Index] * Weight;
                WeightSum += Weight;
            }

            Smoothed.Add(Sum / WeightSum);
        }

        PreCalculatedOvertakeOffsets = MoveTemp(Smoothed);
    }
}

float ATurboRacingSpline::CalculateIdealOffset(float Distance) const
{
    if (!Spline)
    {
        return 0.0f;
    }

    float MaxOffset = (TrackWidth * 0.5f) * TrackWidthUsage;

    float DistBehind = WrapDistance(Distance - ApproachSampleDistance);
    float DistAhead = WrapDistance(Distance + ApproachSampleDistance);

    float CurvatureBehind = GetCurvatureAtDistance(DistBehind, CurvatureSampleRange);
    float CurvatureCurrent = GetCurvatureAtDistance(Distance, CurvatureSampleRange);
    float CurvatureAhead = GetCurvatureAtDistance(DistAhead, CurvatureSampleRange);

    // Not in or near a corner - look ahead for upcoming turns
    if (CurvatureCurrent < RacingLineMinCurvature && CurvatureAhead < RacingLineMinCurvature)
    {
        for (float LookAhead = ApproachSampleDistance; LookAhead < RacingLineLookahead; LookAhead += LookaheadStepSize)
        {
            float LookDist = WrapDistance(Distance + LookAhead);
            float LookCurvature = GetCurvatureAtDistance(LookDist, CurvatureSampleRange);

            if (LookCurvature > RacingLineMinCurvature)
            {
                float TurnSign = GetTurnSign(LookDist, TurnSignLookahead);
                float OffsetMagnitude = FMath::Min(LookCurvature * CurvatureToOffsetScale * TrackWidthUsage, MaxOffset);
                return -TurnSign * OffsetMagnitude;
            }
        }
        return 0.0f;
    }

    float TurnSign = GetTurnSign(Distance, TurnSignLookahead);
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

float ATurboRacingSpline::WrapDistance(float Distance) const
{
    if (!Spline)
    {
        return Distance;
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

    return Distance;
}

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
    if (!Spline)
    {
        return 0.0f;
    }

    float HalfRange = SampleRange * 0.5f;
    FVector TangentA = Spline->GetDirectionAtDistanceAlongSpline(Distance - HalfRange, ESplineCoordinateSpace::World);
    FVector TangentB = Spline->GetDirectionAtDistanceAlongSpline(Distance + HalfRange, ESplineCoordinateSpace::World);

    float Dot = FVector::DotProduct(TangentA, TangentB);
    float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));

    return Angle / SampleRange;
}

float ATurboRacingSpline::GetCurvatureNormalized(float Distance, float SampleRange) const
{
    if (!Spline)
    {
        return 0.0f;
    }

    FVector DirA = Spline->GetDirectionAtDistanceAlongSpline(Distance - SampleRange, ESplineCoordinateSpace::World);
    FVector DirB = Spline->GetDirectionAtDistanceAlongSpline(Distance + SampleRange, ESplineCoordinateSpace::World);
    float Dot = FVector::DotProduct(DirA, DirB);

    return FMath::Clamp(1.0f - Dot, 0.0f, 1.0f);
}

float ATurboRacingSpline::GetTurnSign(float Distance, float InLookaheadDistance) const
{
    if (!Spline)
    {
        return 0.0f;
    }

    FVector RightVec = Spline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector CurrentPos = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector FuturePos = Spline->GetLocationAtDistanceAlongSpline(Distance + InLookaheadDistance, ESplineCoordinateSpace::World);
    FVector Delta = (FuturePos - CurrentPos).GetSafeNormal();

    float DotResult = FVector::DotProduct(Delta, RightVec);

    if (FMath::Abs(DotResult) < 0.01f)
    {
        return 0.0f;
    }

    return (DotResult > 0.0f) ? 1.0f : -1.0f;
}

// =============================================================================
// OVERTAKE LANE
// =============================================================================

float ATurboRacingSpline::GetOvertakeLaneOffset(float Distance) const
{
    if (!bRacingLineCalculated || PreCalculatedOvertakeOffsets.Num() == 0 || !Spline)
    {
        return 0.0f;
    }

    Distance = WrapDistance(Distance);

    float IndexFloat = Distance / RacingLineSampleInterval;
    int32 Index = FMath::FloorToInt(IndexFloat);
    float Alpha = IndexFloat - Index;

    Index = FMath::Clamp(Index, 0, PreCalculatedOvertakeOffsets.Num() - 1);
    int32 NextIndex = (Index + 1) % PreCalculatedOvertakeOffsets.Num();

    return FMath::Lerp(PreCalculatedOvertakeOffsets[Index], PreCalculatedOvertakeOffsets[NextIndex], Alpha);
}

FVector ATurboRacingSpline::GetPointOnOvertakeLane(float Distance) const
{
    if (!Spline)
    {
        return FVector::ZeroVector;
    }

    Distance = WrapDistance(Distance);

    FVector CenterPoint = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

    float Offset = GetOvertakeLaneOffset(Distance);
    if (FMath::Abs(Offset) < 1.0f)
    {
        return CenterPoint;
    }

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    return CenterPoint + (Right * -Offset);
}

void ATurboRacingSpline::DrawDebugOvertakeLane(UWorld* World) const
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
        FVector Point = GetPointOnOvertakeLane(Dist) + FVector(0.0f, 0.0f, 20.0f);

        DrawDebugPoint(World, Point, 8.0f, FColor::Cyan, false, 0.0f);

        if (!bFirstPoint)
        {
            DrawDebugLine(World, PreviousPoint, Point, FColor::Cyan, false, 0.0f, 0, 2.0f);
        }

        PreviousPoint = Point;
        bFirstPoint = false;
    }

    if (Spline->IsClosedLoop() && !bFirstPoint)
    {
        FVector FirstPoint = GetPointOnOvertakeLane(0.0f) + FVector(0.0f, 0.0f, 20.0f);
        DrawDebugLine(World, PreviousPoint, FirstPoint, FColor::Cyan, false, 0.0f, 0, 2.0f);
    }
}