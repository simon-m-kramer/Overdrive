// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TurboVehicle.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UTurboVehicleDetectionComponent;
class UTurboVehicleData;

UCLASS()
class OVERDRIVE_API ATurboVehicle : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ATurboVehicle();

	virtual void PostInitializeComponents() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TObjectPtr<UTurboVehicleData> VehicleData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString VehicleName = TEXT("Mercedes");

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetSteeringInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetThrottleInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetBrakeInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetHandbrakeInput(bool bEngaged);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetSpeedKmh() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetForwardSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	int32 GetCurrentGear() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetEngineRPM() const;

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	float GetMaxEngineRPM() const;

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UTurboVehicleDetectionComponent* GetDetectionComponent() const { return DetectionComponent; }

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UChaosWheeledVehicleMovementComponent* GetVehicleMovementComponent() const { return VehicleMovement; }

protected:
	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

	void SetupDefaultWheels();
	void SetupDefaultEngine();
	void SetupDefaultTransmission();
	void SetupDefaultSteering();

	void ApplyEngine();
	void ApplyTransmission();
	void ApplySteering();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UBoxComponent> DetectionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UTurboVehicleDetectionComponent> DetectionComponent;

};
