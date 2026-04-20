// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIVehicle.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ATurboAIVehicle::ATurboAIVehicle()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ATurboAIVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

void ATurboAIVehicle::BeginPlay()
{
    Super::BeginPlay();

}

