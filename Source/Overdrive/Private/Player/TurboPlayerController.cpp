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
#include "AI/TurboAIController.h"
#include "ChaosWheeledVehicleMovementComponent.h"


void ATurboPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Setup Input Mapping Context
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

    // Mouse Settings
    SetShowMouseCursor(false);
    SetInputMode(FInputModeGameOnly());

    // Create Pause Menu Instance
    if (HUDWidgetClass && IsLocalController())
    {
        HUDWidget = CreateWidget<UTurboHUDWidget>(this, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
        }
    }

    // Bind Race Result Screen to Race Manager Delegate
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

    ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode || !GameMode->GetRaceManager())
    {
        return;
    }

    UTurboRaceManager* RaceManager = GameMode->GetRaceManager();

    // Unbind so it doesn't fire again
    RaceManager->OnVehicleFinished.RemoveDynamic(this, &ATurboPlayerController::OnVehicleFinished);

    if (!RaceResultClass || !IsLocalController())
    {
        return;
    }

    HandOffToAI();

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

void ATurboPlayerController::HandOffToAI()
{
    if (!PlayerVehicle || !PostRaceAIClass)
    {
        return;
    }

    APawn* VehiclePawn = PlayerVehicle;

    // Disable auto-management so UnPossess doesn't snap the camera away
    this->bAutoManageActiveCameraTarget = false;

    //if (PlayerVehicle->GetVehicleMovementComponent())
    //{
    //    PlayerVehicle->GetVehicleMovementComponent()->SetThrottleInput(1.0f);
    //    PlayerVehicle->GetVehicleMovementComponent()->SetBrakeInput(0.0f);
    //}

    UnPossess();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ATurboAIController* AIController = GetWorld()->SpawnActor<ATurboAIController>(PostRaceAIClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (AIController)
    {
        AIController->Possess(VehiclePawn);
    }

    // Keep the camera following the vehicle
    SetViewTargetWithBlend(VehiclePawn, 0.0f);
}