// Copyright Simon Kramer. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "TurboVehicle.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UTurboVehicleDetectionComponent;


UCLASS()
class OVERDRIVE_API ATurboVehicle : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ATurboVehicle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString VehicleName = TEXT("Car");

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

	// =========================================================================
	// PERFORMANCE STATS
	// =========================================================================

	/** Maximum speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float MaxSpeedCms = 5000.0f;  // 5000 = ~180 km/h, 4166 = 150km/h

	/** Lateral grip as acceleration in cm/s² (cornering capability) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float LateralGripCms2 = 2400.0f;  // 1800.0f

	/** Braking deceleration in cm/s2 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float BrakeDecelerationCms2 = 2000.0f;  // 2000.0f

	/** Forward acceleration in cm/s2 (approximate average) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float AccelerationCms2 = 1200.0f;  // 800.0f


protected:
	UPROPERTY()
	TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;

	void SetupWheels();
	void SetupEngine();
	void SetupTransmission();
	void SetupSteering();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UBoxComponent> DetectionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	TObjectPtr<UTurboVehicleDetectionComponent> DetectionComponent;

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

};