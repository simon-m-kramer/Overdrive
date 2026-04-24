// Copyright Simon Kramer. All Rights Reserved.


#include "Player/TurboSpectatorController.h"
#include "AI/TurboAIVehicle.h"
#include "Kismet/GameplayStatics.h"

void ATurboSpectatorController::BeginPlay()
{
    Super::BeginPlay();

    SetShowMouseCursor(false);
    SetInputMode(FInputModeGameOnly());

    ATurboAIVehicle* Vehicle = TargetVehicle;

    // Fallback: find any AI vehicle if none was assigned
    if (!Vehicle)
    {
        Vehicle = Cast<ATurboAIVehicle>(UGameplayStatics::GetActorOfClass(GetWorld(), ATurboAIVehicle::StaticClass()));
    }

    if (Vehicle)
    {
        SetViewTargetWithBlend(Vehicle, 0.5f);
    }
}