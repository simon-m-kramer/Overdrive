// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboTachometerWidget.generated.h"

class UCommonTextBlock;
class UImage;
class ATurboVehicle;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboTachometerWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Gauge;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Needle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Gear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tachometer")
	float NeedleAngleMin = -115.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tachometer")
	float NeedleAngleMax = 115.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tachometer")
	float NeedleInterpSpeed = 10.0f;

private:
	TWeakObjectPtr<ATurboVehicle> CachedVehicle;
	float CurrentNeedleAngle = 0.0f;

	FString GearToString(int32 Gear) const;

};
