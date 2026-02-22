// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboVehicle.h"
#include "TurboAIVehicle.generated.h"


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

};
