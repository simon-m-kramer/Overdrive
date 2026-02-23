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
#include "Kismet/GameplayStatics.h"


void ATurboPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShouldPerformFullTickWhenPaused = true;

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
    // Unpausing
    if (PauseMenuWidget && PauseMenuWidget->IsActivated())
    {
        PauseMenuWidget->DeactivateWidget();
        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
        SetPause(false);
        PauseMenuWidget->HideMenu();
        return;
    }

    // Pausing
    if (IsLocalController())
    {
        if (!PauseMenuWidget && PauseMenuClass)
        {
            PauseMenuWidget = CreateWidget<UTurboPauseMenuWidget>(this, PauseMenuClass);
            if (PauseMenuWidget)
            {
                PauseMenuWidget->AddToViewport(100);
            }
        }

        if (PauseMenuWidget)
        {
            PauseMenuWidget->ActivateWidget();
            SetInputMode(FInputModeGameAndUI());
            bShowMouseCursor = true;
            SetPause(true);
            PauseMenuWidget->ShowMenu();
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

    // Hide Player HUD
    if (HUDWidget)
    {
        HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ATurboPlayerController::HandOffToAI()
{
    if (!PlayerVehicle || !PostRaceAIClass)
    {
        return;
    }

    APawn* VehiclePawn = PlayerVehicle;

    // Cache the vehicle's current velocity before unpossessing
    FVector SavedVelocity = VehiclePawn->GetVelocity();
    FVector SavedAngularVelocity = FVector::ZeroVector;
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(VehiclePawn->GetRootComponent()))
    {
        SavedAngularVelocity = PrimComp->GetPhysicsAngularVelocityInDegrees();
    }

    // Disable auto-management so UnPossess doesn't snap the camera away
    this->bAutoManageActiveCameraTarget = false;

    UnPossess();

    // Spawn new AI controller and have it possess the car
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ATurboAIController* AIController = GetWorld()->SpawnActor<ATurboAIController>(PostRaceAIClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (AIController)
    {
        AIController->Possess(VehiclePawn);
    }

    // Restore the velocity that got wiped during the possession swap
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(VehiclePawn->GetRootComponent()))
    {
        PrimComp->SetPhysicsLinearVelocity(SavedVelocity);
        PrimComp->SetPhysicsAngularVelocityInDegrees(SavedAngularVelocity);
    }

    // Keep the camera following the vehicle
    SetViewTargetWithBlend(VehiclePawn, 0.0f);
}

