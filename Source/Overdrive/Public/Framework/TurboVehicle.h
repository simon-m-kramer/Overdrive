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

	virtual void Tick(float DeltaTime) override;


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

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UTurboVehicleDetectionComponent* GetDetectionComponent() const { return DetectionComponent; }

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

	UPROPERTY(EditAnywhere, Category = "Surface")
	float OffTrackDragForce = 2000000.0f;  // 800000.0f

private:
	EPhysicalSurface GetWheelSurfaceType() const;

};