// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIVehicle.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/TurboVehicleDetectionComponent.h"


ATurboAIVehicle::ATurboAIVehicle()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	DetectionComponent = CreateDefaultSubobject<UTurboVehicleDetectionComponent>(TEXT("DetectionComponent"));
}

void ATurboAIVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

