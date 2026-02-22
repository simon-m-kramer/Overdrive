// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TurboPlayerController.generated.h"

class UInputMappingContext;
class ATurboPlayerVehicle;
class UTurboHUDWidget;
class UTurboPauseMenuWidget;
class UInputAction;
class UTurboRaceResultWidget;
class ATurboAIController;


UCLASS()
class OVERDRIVE_API ATurboPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Input")
    TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> PauseAction;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UTurboHUDWidget> HUDWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UTurboPauseMenuWidget> PauseMenuClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UTurboRaceResultWidget> RaceResultClass;

    UPROPERTY(EditAnywhere, Category = "AI")
    TSubclassOf<ATurboAIController> PostRaceAIClass;



    UPROPERTY()
    TObjectPtr<ATurboPlayerVehicle> PlayerVehicle;

    UPROPERTY()
    TObjectPtr<UTurboHUDWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UTurboPauseMenuWidget> PauseMenuWidget;

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;

    void TogglePauseMenu();
    void HandOffToAI();

    UFUNCTION()
    void OnVehicleFinished(ATurboVehicle* Vehicle);
};
