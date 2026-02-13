// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIVehicle.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"


ATurboAIVehicle::ATurboAIVehicle()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ATurboAIVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

