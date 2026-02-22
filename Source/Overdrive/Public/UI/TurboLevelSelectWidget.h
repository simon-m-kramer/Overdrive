// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboLevelEntry.h"
#include "TurboLevelSelectWidget.generated.h"

class UCommonButtonBase;
class UTurboLevelSelectEntryWidget;
class UHorizontalBox;

UCLASS()
class OVERDRIVE_API UTurboLevelSelectWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_Levels;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Start;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Back;

	UPROPERTY(EditAnywhere, Category = "Level Select")
	TArray<FTurboLevelEntry> Levels;

	UPROPERTY(EditAnywhere, Category = "Level Select")
	TSubclassOf<UTurboLevelSelectEntryWidget> EntryWidgetClass;

private:
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnBackClicked();

	void OnLevelSelected(UTurboLevelSelectEntryWidget* Clicked);

	UPROPERTY()
	TArray<TObjectPtr<UTurboLevelSelectEntryWidget>> EntryWidgets;

	UPROPERTY()
	TObjectPtr<UTurboLevelSelectEntryWidget> SelectedEntry;
};