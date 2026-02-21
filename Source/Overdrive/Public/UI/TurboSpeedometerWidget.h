// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboSpeedometerWidget.generated.h"

class UCommonTextBlock;
class UImage;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboSpeedometerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Gauge;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Needle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Speed;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer")
	float MaxDisplaySpeed = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer")
	float NeedleAngleMin = -115.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer")
	float NeedleAngleMax = 115.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speedometer")
	float NeedleInterpSpeed = 8.0f;

private:
	TWeakObjectPtr<ATurboVehicle> CachedVehicle;
	float CurrentNeedleAngle = 0.0f;
};
