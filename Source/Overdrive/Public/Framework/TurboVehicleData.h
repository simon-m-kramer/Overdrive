// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TurboVehicleData.generated.h"

class UChaosVehicleWheel;


// Per-wheel configuration
USTRUCT(BlueprintType)
struct FTurboWheelSetup
{
	GENERATED_BODY()

	// Bone name in the skeleton that this wheel maps to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	// Optional: separate wheel mesh (leave null if wheels are baked into the skeletal mesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WheelMesh;

	// Rotation offset for the wheel mesh (e.g., flip left vs right side)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "WheelMesh != nullptr"))
	FRotator WheelMeshRotation = FRotator::ZeroRotator;
};

// Engine configuration
USTRUCT(BlueprintType)
struct FTurboEngineSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTorque = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRPM = 7000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleRPM = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineBrakeEffect = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineRevUpMOI = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineRevDownRate = 600.0f;

	// Torque curve as (RPM, NormalizedTorque) pairs
	// If empty, a sensible default curve is generated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> TorqueCurveKeys;
};

// Transmission configuration
USTRUCT(BlueprintType)
struct FTurboTransmissionSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomaticGears = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoReverse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalRatio = 2.81f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChangeUpRPM = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChangeDownRPM = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GearChangeTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransmissionEfficiency = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ForwardGearRatios;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ReverseGearRatios;
};

// Steering configuration
USTRUCT(BlueprintType)
struct FTurboSteeringSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleRatio = 0.7f;

	// Steering curve as (Speed km/h, SteeringMultiplier) pairs
	// If empty, a sensible default is generated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> SteeringCurveKeys;
};

/**
 * Data asset that fully describes a vehicle's configuration.
 * Create one of these per car model in the Content Browser.
 */
UCLASS()
class OVERDRIVE_API UTurboVehicleData : public UDataAsset
{
	GENERATED_BODY()
	

public:
	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString VehicleName = TEXT("Car");

	// --- Mesh Setup ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<USkeletalMesh> VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSubclassOf<UAnimInstance> AnimBlueprintClass;

	// Optional: separate chassis mesh (leave null if chassis is part of skeletal mesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> ChassisMesh;

	// Bone to attach the chassis mesh to (only used if ChassisMesh is set)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (EditCondition = "ChassisMesh != nullptr"))
	FName ChassisBoneName = FName("Root");

	// Optional: separate glass mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> GlassMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (EditCondition = "GlassMesh != nullptr"))
	FName GlassBoneName = FName("Root");

	// --- Wheels ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheels")
	TArray<FTurboWheelSetup> Wheels;

	// --- Physics ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float ChassisHeight = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DragCoefficient = 0.31f;

	// --- Mechanical ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboEngineSetup Engine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboTransmissionSetup Transmission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboSteeringSetup Steering;

	// --- Helpers ---

	// Returns true if this vehicle uses separate static mesh components for wheels
	bool UsesSeparateWheelMeshes() const
	{
		for (const FTurboWheelSetup& Wheel : Wheels)
		{
			if (!Wheel.WheelMesh.IsNull())
			{
				return true;
			}
		}
		return false;
	}

	bool UsesSeparateChassisMesh() const { return !ChassisMesh.IsNull(); }
	bool UsesSeparateGlassMesh() const { return !GlassMesh.IsNull(); }

};
