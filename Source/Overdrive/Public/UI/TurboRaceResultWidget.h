// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboRaceResultWidget.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;

UCLASS()
class OVERDRIVE_API UTurboRaceResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	void SetResult(int32 Position, int32 TotalEntries, float BestLapTime);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_Position;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_BestLapTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_MainMenu;

private:
	UFUNCTION()
	void OnMainMenuClicked();
};