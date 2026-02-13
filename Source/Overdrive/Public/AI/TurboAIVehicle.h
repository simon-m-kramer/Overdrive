// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboVehicle.h"
#include "TurboAIVehicle.generated.h"

class UTurboVehicleDetectionComponent;

/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboAIVehicle : public ATurboVehicle
{
	GENERATED_BODY()
	

public:
	ATurboAIVehicle();

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UTurboVehicleDetectionComponent* GetDetectionComponent() const { return DetectionComponent; }

protected:
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UTurboVehicleDetectionComponent> DetectionComponent;

};
