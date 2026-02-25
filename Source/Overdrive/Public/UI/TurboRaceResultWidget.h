// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboRaceResultWidget.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;
class UVerticalBox;
class UTurboStandingsEntryWidget;
class UTurboRaceManager;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboRaceResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	void SetResult(UTurboRaceManager* RaceManager, ATurboVehicle* PlayerVehicle);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_Position;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Txt_BestLapTime;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VBox_Results;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_MainMenu;

	UPROPERTY(EditAnywhere, Category = "Race Result")
	TSubclassOf<UTurboStandingsEntryWidget> EntryWidgetClass;

private:
	UFUNCTION()
	void OnMainMenuClicked();
};