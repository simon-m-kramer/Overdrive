// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TurboDrivingProfile.generated.h"

/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboDrivingProfile : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, Category = "Performance")
    float MaxSpeedCms = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float LateralGripCms2 = 2300.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float BrakeDecelerationCms2 = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float AccelerationCms2 = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float CorneringSpeedSafetyFactor = 1.0f;

};
