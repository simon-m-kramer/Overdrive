// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TurboPlayerController.generated.h"

class UInputMappingContext;
class ATurboPlayerVehicle;

UCLASS()
class OVERDRIVE_API ATurboPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = "Input")
    TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

    UPROPERTY()
    TObjectPtr<ATurboPlayerVehicle> PlayerVehicle;

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
};
