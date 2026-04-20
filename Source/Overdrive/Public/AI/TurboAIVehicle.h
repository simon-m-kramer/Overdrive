// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboVehicle.h"
#include "TurboAIVehicle.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboAIVehicle : public ATurboVehicle
{
	GENERATED_BODY()
	

public:
	ATurboAIVehicle();

protected:
	virtual void PossessedBy(AController* NewController) override;

	virtual void BeginPlay() override;
};
