// Copyright Simon Kramer. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboFuelWidget.generated.h"

class UImage;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboFuelWidget : public UCommonUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> Img_Needle;

    UPROPERTY(EditAnywhere, Category = "Fuel")
    float NeedleAngleMin = -60.0f;

    UPROPERTY(EditAnywhere, Category = "Fuel")
    float NeedleAngleMax = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Fuel")
    float NeedleInterpSpeed = 10.0f;

    /** Base drain per second at idle. */
    UPROPERTY(EditAnywhere, Category = "Fuel")
    float BaseDrainRate = 0.0003f;

    /** Extra drain multiplier at max RPM. */
    UPROPERTY(EditAnywhere, Category = "Fuel")
    float RPMDrainMultiplier = 3.0f;

private:
    TWeakObjectPtr<ATurboVehicle> CachedVehicle;
    float CurrentNeedleAngle = 0.0f;
    float FuelLevel = 1.0f;
};