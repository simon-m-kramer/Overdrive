// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"



ATurboRacingSpline::ATurboRacingSpline()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("RacingSpline"));
	SetRootComponent(Spline);

	GameplayTags.AddTag(TurboGameplayTags::Track_MainSpline);

}

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

// Helper for speed calculations
float ATurboRacingSpline::GetTargetSpeedAtDistance(float Distance, float MaxSpeed, float GripFactor) const
{
    float Curvature = GetCurvatureAtDistance(Distance, 200.0f); // tune sample range
    if (Curvature < KINDA_SMALL_NUMBER) return MaxSpeed;

    // v = sqrt(grip / curvature) - classic circular motion
    return FMath::Min(MaxSpeed, FMath::Sqrt(GripFactor / Curvature));
}

void ATurboRacingSpline::BeginPlay()
{
	Super::BeginPlay();
	
}



