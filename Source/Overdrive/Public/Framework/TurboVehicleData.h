// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TurboVehicleData.generated.h"



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

USTRUCT(BlueprintType)
struct FTurboSteeringSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> SteeringCurveKeys;
};


UCLASS()
class OVERDRIVE_API UTurboVehicleData : public UDataAsset
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float ChassisHeight = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DragCoefficient = 0.31f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboEngineSetup Engine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboTransmissionSetup Transmission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mechanical")
	FTurboSteeringSetup Steering;

};
