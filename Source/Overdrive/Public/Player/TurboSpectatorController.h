// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TurboSpectatorController.generated.h"

class ATurboAIVehicle;

UCLASS()
class OVERDRIVE_API ATurboSpectatorController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
    TObjectPtr<ATurboAIVehicle> TargetVehicle;
};