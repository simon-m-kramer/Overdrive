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

	CalculateMaxCurvature();
}

// =============================================================================
// SPLINE QUERIES
// =============================================================================

FVector ATurboRacingSpline::GetLocationAtDistance(float Distance) const
{
	if (!Spline) return FVector::ZeroVector;

	Distance = WrapDistance(Distance);
	return Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector ATurboRacingSpline::GetDirectionAtDistance(float Distance) const
{
	if (!Spline) return FVector::ForwardVector;

	Distance = WrapDistance(Distance);
	return Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float ATurboRacingSpline::GetSplineLength() const
{
	return Spline ? Spline->GetSplineLength() : 0.0f;
}

bool ATurboRacingSpline::IsClosedLoop() const
{
	return Spline ? Spline->IsClosedLoop() : false;
}

// =============================================================================
// CURVATURE ANALYSIS
// =============================================================================

void ATurboRacingSpline::CalculateMaxCurvature()
{
	if (!Spline) return;

	const float SplineLength = Spline->GetSplineLength();
	const float SampleInterval = 100.0f;

	MaxTrackCurvature = 0.0f;

	for (float Dist = 0.0f; Dist < SplineLength; Dist += SampleInterval)
	{
		const float Curvature = GetCurvatureAtDistance(Dist, CurvatureSampleRange);
		MaxTrackCurvature = FMath::Max(MaxTrackCurvature, Curvature);
	}
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
	const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));

	return Angle / SampleRange;
}

float ATurboRacingSpline::GetTurnSign(float Distance, float InLookaheadDistance) const
{
	if (!Spline) return 0.0f;

	const float HalfRange = InLookaheadDistance * 0.5f;

	const FVector TangentA = Spline->GetDirectionAtDistanceAlongSpline(
		WrapDistance(Distance - HalfRange), ESplineCoordinateSpace::World);
	const FVector TangentB = Spline->GetDirectionAtDistanceAlongSpline(
		WrapDistance(Distance + HalfRange), ESplineCoordinateSpace::World);
	const FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(
		WrapDistance(Distance), ESplineCoordinateSpace::World);

	const FVector Cross = FVector::CrossProduct(TangentA, TangentB);
	const float DotUp = FVector::DotProduct(Cross, Up);

	if (FMath::Abs(DotUp) < KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return (DotUp > 0.0f) ? 1.0f : -1.0f;
}

// =============================================================================
// UTILITY
// =============================================================================

float ATurboRacingSpline::WrapDistance(float Distance) const
{
	if (!Spline) return Distance;

	const float SplineLength = Spline->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER) return 0.0f;

	if (Spline->IsClosedLoop())
	{
		Distance = FMath::Fmod(Distance, SplineLength);
		if (Distance < 0.0f) Distance += SplineLength;
	}
	else
	{
		Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);
	}

	return Distance;
}

// =============================================================================
// DEBUG
// =============================================================================

void ATurboRacingSpline::DrawDebugRacingLine(UWorld* World) const
{
	if (!Spline || !World) return;

	const float SplineLength = Spline->GetSplineLength();
	const float DrawInterval = 100.0f;
	const float DebugHeight = 20.0f;

	FVector PreviousPoint = FVector::ZeroVector;
	bool bFirstPoint = true;

	for (float Dist = 0.0f; Dist < SplineLength; Dist += DrawInterval)
	{
		const FVector Point = GetLocationAtDistance(Dist) + FVector(0.0f, 0.0f, DebugHeight);

		// Color by curvature
		const float Curvature = GetCurvatureAtDistance(Dist, CurvatureSampleRange);
		const float NormalizedCurvature = (MaxTrackCurvature > KINDA_SMALL_NUMBER)
			? FMath::Clamp(Curvature / MaxTrackCurvature, 0.0f, 1.0f)
			: 0.0f;

		// Green = straight, Yellow = mid curve, Red = tight curve
		const FColor PointColor = FColor::MakeRedToGreenColorFromScalar(1.0f - NormalizedCurvature);

		DrawDebugPoint(World, Point, 8.0f, PointColor, false, 0.0f);

		if (!bFirstPoint)
		{
			DrawDebugLine(World, PreviousPoint, Point, PointColor, false, 0.0f, 0, 2.0f);
		}

		PreviousPoint = Point;
		bFirstPoint = false;
	}

	if (IsClosedLoop() && !bFirstPoint)
	{
		const FVector FirstPoint = GetLocationAtDistance(0.0f) + FVector(0.0f, 0.0f, DebugHeight);
		DrawDebugLine(World, PreviousPoint, FirstPoint, FColor::Green, false, 0.0f, 0, 2.0f);
	}
}