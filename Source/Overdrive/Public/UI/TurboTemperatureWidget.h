// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboTemperatureWidget.generated.h"

class UImage;
class ATurboVehicle;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboTemperatureWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_Needle;

    UPROPERTY(EditAnywhere, Category = "Temperature")
    float NeedleAngleMin = -60.0f;

    UPROPERTY(EditAnywhere, Category = "Temperature")
    float NeedleAngleMax = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Temperature")
    float NeedleInterpSpeed = 10.0f;

    /** Resting point on the gauge (0-1). */
    UPROPERTY(EditAnywhere, Category = "Temperature", meta = (ClampMin = "0", ClampMax = "1"))
    float NormalTemperature = 0.45f;

    UPROPERTY(EditAnywhere, Category = "Temperature")
    float HeatRate = 0.04f;

    UPROPERTY(EditAnywhere, Category = "Temperature")
    float CoolRate = 0.06f;

    /** RPM ratio above which the engine is considered hot. */
    UPROPERTY(EditAnywhere, Category = "Temperature", meta = (ClampMin = "0", ClampMax = "1"))
    float HotRPMThreshold = 0.7f;

private:
    TWeakObjectPtr<ATurboVehicle> CachedVehicle;
    float CurrentNeedleAngle = 0.0f;
    float SimulatedTemp = 0.45f;
};
