// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

ATurboRacingSpline::ATurboRacingSpline()
{
	GameplayTags.AddTag(TurboGameplayTags::Track_MainSpline);
}

void ATurboRacingSpline::BeginPlay()
{
	Super::BeginPlay();
	CalculateMaxCurvature();
}

FVector ATurboRacingSpline::GetLocationAtDistance(float Distance) const
{
	if (!RacingLineSpline) return FVector::ZeroVector;

	Distance = WrapDistance(Distance);
	return RacingLineSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector ATurboRacingSpline::GetDirectionAtDistance(float Distance) const
{
	if (!RacingLineSpline) return FVector::ForwardVector;

	Distance = WrapDistance(Distance);
	return RacingLineSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float ATurboRacingSpline::GetSplineLength() const
{
	return RacingLineSpline ? RacingLineSpline->GetSplineLength() : 0.0f;
}

float ATurboRacingSpline::GetCurvatureAtDistance(float Distance, float SampleRange) const
{
	if (!RacingLineSpline) return 0.0f;

	const float HalfRange = SampleRange * 0.5f;

	const float DistA = WrapDistance(Distance - HalfRange);
	const float DistB = WrapDistance(Distance + HalfRange);

	const FVector TangentA = RacingLineSpline->GetDirectionAtDistanceAlongSpline(DistA, ESplineCoordinateSpace::World);
	const FVector TangentB = RacingLineSpline->GetDirectionAtDistanceAlongSpline(DistB, ESplineCoordinateSpace::World);

	const float Dot = FVector::DotProduct(TangentA, TangentB);
	const float Angle = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));

	return Angle / SampleRange;
}

float ATurboRacingSpline::GetTurnSign(float Distance, float InLookaheadDistance) const
{
	if (!RacingLineSpline) return 0.0f;

	const float HalfRange = InLookaheadDistance * 0.5f;

	const FVector TangentA = RacingLineSpline->GetDirectionAtDistanceAlongSpline(WrapDistance(Distance - HalfRange), ESplineCoordinateSpace::World);
	const FVector TangentB = RacingLineSpline->GetDirectionAtDistanceAlongSpline(WrapDistance(Distance + HalfRange), ESplineCoordinateSpace::World);
	const FVector Up = RacingLineSpline->GetUpVectorAtDistanceAlongSpline(WrapDistance(Distance), ESplineCoordinateSpace::World);

	const FVector Cross = FVector::CrossProduct(TangentA, TangentB);
	const float DotUp = FVector::DotProduct(Cross, Up);

	if (FMath::Abs(DotUp) < KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return (DotUp > 0.0f) ? 1.0f : -1.0f;
}



float ATurboRacingSpline::WrapDistance(float Distance) const
{
	if (!RacingLineSpline) return Distance;

	const float Length = RacingLineSpline->GetSplineLength();
	if (Length <= KINDA_SMALL_NUMBER) return 0.0f;

	if (bClosedLoop)
	{
		Distance = FMath::Fmod(Distance, Length);
		if (Distance < 0.0f) Distance += Length;
	}
	else
	{
		Distance = FMath::Clamp(Distance, 0.0f, Length - KINDA_SMALL_NUMBER);
	}

	return Distance;
}

void ATurboRacingSpline::CalculateMaxCurvature(float SampleInterval, float SampleRange)
{
	if (!RacingLineSpline) return;

	const float Length = RacingLineSpline->GetSplineLength();

	MaxTrackCurvature = 0.0f;

	for (float Dist = 0.0f; Dist < Length; Dist += SampleInterval)
	{
		const float Curvature = GetCurvatureAtDistance(Dist, SampleRange);
		MaxTrackCurvature = FMath::Max(MaxTrackCurvature, Curvature);
	}
}

