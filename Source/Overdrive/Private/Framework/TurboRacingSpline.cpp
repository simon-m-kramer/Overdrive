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

    FVector DirA = Spline->GetDirectionAtDistanceAlongSpline(Distance - SampleRange, ESplineCoordinateSpace::World);
    FVector DirB = Spline->GetDirectionAtDistanceAlongSpline(Distance + SampleRange, ESplineCoordinateSpace::World);

    float Dot = FVector::DotProduct(DirA, DirB);
    float Curvature = FMath::Clamp(1.0f - Dot, 0.0f, 1.0f);

    // Debug: Show actual angle in degrees
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
    UE_LOG(LogTemp, Warning, TEXT("Curvature at %.0f: Angle=%.1f°, Curvature=%.4f (SampleRange=%.0f)"),
        Distance, AngleDegrees, Curvature, SampleRange);

    return Curvature;

    /*
    if (!Spline) return 0.0f;

    // Get directions at two points around the target distance
    FVector DirA = Spline->GetDirectionAtDistanceAlongSpline(Distance - SampleRange, ESplineCoordinateSpace::World);
    FVector DirB = Spline->GetDirectionAtDistanceAlongSpline(Distance + SampleRange, ESplineCoordinateSpace::World);

    // The Dot Product tells us how parallel they are. 
    // 1.0 = Same direction. < 1.0 = the path is turning.
    float Dot = FVector::DotProduct(DirA, DirB);

    // Normalize to a 0-1 scale where 1 is a 90-degree difference
    return FMath::Clamp(1.0f - Dot, 0.0f, 1.0f);
    */
}

float ATurboRacingSpline::GetTurnSign(float Distance) const
{
    FVector Tangent = Spline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Up = FVector::UpVector;
    FVector Right = FVector::CrossProduct(Up, Tangent);

    // Sample a point slightly ahead
    FVector FuturePos = Spline->GetLocationAtDistanceAlongSpline(Distance + 200.f, ESplineCoordinateSpace::World);
    FVector CurrentPos = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Delta = (FuturePos - CurrentPos).GetSafeNormal();

    return (FVector::DotProduct(Delta, Right) > 0) ? 1.0f : -1.0f;
}

void ATurboRacingSpline::BeginPlay()
{
	Super::BeginPlay();
	
}



