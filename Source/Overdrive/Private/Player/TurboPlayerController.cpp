// Copyright Simon Kramer. All Rights Reserved.


#include "Player/TurboPlayerController.h"
#include "Player/TurboPlayerVehicle.h"
#include "UI/TurboHUDWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UI/TurboPauseMenuWidget.h"
#include "UI/TurboRaceResultWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboGameMode.h"

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

    if (HUDWidgetClass && IsLocalController())
    {
        HUDWidget = CreateWidget<UTurboHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }

    if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (UTurboRaceManager* RaceManager = GameMode->GetRaceManager())
        {
            RaceManager->OnVehicleFinished.AddDynamic(this, &ATurboPlayerController::OnVehicleFinished);
        }
    }
}

void ATurboPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (PauseAction)
        {
            Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ATurboPlayerController::TogglePauseMenu);
        }
    }
}

void ATurboPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PlayerVehicle = Cast<ATurboPlayerVehicle>(InPawn);

}

void ATurboPlayerController::TogglePauseMenu()
{
    if (PauseMenuWidget && PauseMenuWidget->IsActivated())
    {
        PauseMenuWidget->DeactivateWidget();
        return;
    }

    if (PauseMenuClass && IsLocalController())
    {
        PauseMenuWidget = CreateWidget<UTurboPauseMenuWidget>(this, PauseMenuClass);
        if (PauseMenuWidget)
        {
            PauseMenuWidget->AddToViewport(100); // Above HUD
            PauseMenuWidget->ActivateWidget();
        }
    }
}

void ATurboPlayerController::OnVehicleFinished(ATurboVehicle* Vehicle)
{
    if (Vehicle != PlayerVehicle)
    {
        return;
    }

    if (!RaceResultClass || !IsLocalController())
    {
        return;
    }

    ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode || !GameMode->GetRaceManager())
    {
        return;
    }

    UTurboRaceManager* RaceManager = GameMode->GetRaceManager();

    UTurboRaceResultWidget* ResultWidget = CreateWidget<UTurboRaceResultWidget>(this, RaceResultClass);
    if (ResultWidget)
    {
        ResultWidget->SetResult(
            RaceManager->GetPlacement(Vehicle),
            RaceManager->GetEntryCount(),
            RaceManager->GetBestLapTime(Vehicle)
        );
        ResultWidget->AddToViewport(100);
        ResultWidget->ActivateWidget();
    }
}