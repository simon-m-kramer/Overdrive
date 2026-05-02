// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboCountdownWidget.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboCountdownWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_Countdown;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_Pop;

private:
	UFUNCTION()
	void OnCountdownUpdated(int32 Count);

	UFUNCTION()
	void OnRaceStarted();

	UFUNCTION()
	void OnRemoveAnimFinished();
};
