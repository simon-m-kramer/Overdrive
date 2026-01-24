// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TurboVehicle.generated.h"


class UChaosWheeledVehicleMovementComponent;

/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboVehicle : public AWheeledVehiclePawn
{
	GENERATED_BODY()
	

public:
	ATurboVehicle();

	// Input methods
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetSteeringInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetThrottleInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetBrakeInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetHandbrakeInput(bool bEngaged);

	// Getters for AI decision making
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetSpeedKmh() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetForwardSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	FVector GetLookAheadPoint() const;

protected:
	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

	void SetupWheels();
	void SetupEngine();
	void SetupTransmission();
	void SetupSteering();

};
