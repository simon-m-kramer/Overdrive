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

void ATurboRacingSpline::BeginPlay()
{
	Super::BeginPlay();
	
}



