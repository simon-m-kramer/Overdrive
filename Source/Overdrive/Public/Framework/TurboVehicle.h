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

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif

	// The data asset that defines this vehicle's configuration.
	// Set this in the Blueprint defaults or via SpawnActor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TObjectPtr<UTurboVehicleData> VehicleData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString VehicleName = TEXT("Car");

	// --- Input ---

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetSteeringInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetThrottleInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetBrakeInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SetHandbrakeInput(bool bEngaged);

	// --- Queries ---

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

	// --- Constructor-time defaults (Chaos needs wheels before init) ---
	void SetupDefaultWheels();
	void SetupDefaultEngine();
	void SetupDefaultTransmission();
	void SetupDefaultSteering();

	// --- Data asset application ---
	void ApplyMeshes();
	void ApplyWheelMeshes();
	void ApplyWheelSetups();
	void ApplyEngine();
	void ApplyTransmission();
	void ApplySteering();

	// --- Detection ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UBoxComponent> DetectionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UTurboVehicleDetectionComponent> DetectionComponent;

	// --- Visual components (created once, meshes assigned from data asset) ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ChassisMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GlassMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WheelFL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WheelFR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WheelRL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WheelRR;

	// Indexed access to wheel mesh components for data-driven setup
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshComponents;
};
