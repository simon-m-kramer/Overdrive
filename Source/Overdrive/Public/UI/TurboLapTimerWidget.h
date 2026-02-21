// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboLapTimerWidget.generated.h"

class UCommonTextBlock;
class UTurboRaceManager;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboLapTimerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_LapTime;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_BestLapTime;

private:
	TWeakObjectPtr<UTurboRaceManager> CachedRaceManager;
	TWeakObjectPtr<ATurboVehicle> CachedVehicle;
};
