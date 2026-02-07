// Copyright Simon Kramer. All Rights Reserved.


#include "Player/TurboPlayerController.h"
#include "Player/TurboPlayerVehicle.h"
#include "EnhancedInputSubsystems.h"

void ATurboPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        for (UInputMappingContext* Context : InputMappingContexts)
        {
            if (Context)
            {
                Subsystem->AddMappingContext(Context, 0);
            }
        }
    }
}

void ATurboPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    PlayerVehicle = Cast<ATurboPlayerVehicle>(InPawn);
}