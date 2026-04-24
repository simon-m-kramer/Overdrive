// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboLevelSelectWidget.generated.h"

class UCommonButtonBase;
class UWrapBox;
class UTurboLevelData;
class UTurboLevelEntryWidget;

UCLASS(Abstract)
class OVERDRIVE_API UTurboLevelSelectWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> LevelGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Play;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Back;

	UPROPERTY(EditDefaultsOnly, Category = "Level Select")
	TArray<TObjectPtr<UTurboLevelData>> Levels;

	UPROPERTY(EditDefaultsOnly, Category = "Level Select")
	TSubclassOf<UTurboLevelEntryWidget> EntryWidgetClass;

private:
	void PopulateLevelGrid();

	UFUNCTION()
	void OnEntrySelected(UTurboLevelData* InLevelData);

	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnBackClicked();

	void OnLevelLoadedForPlay();

	UPROPERTY()
	TObjectPtr<UTurboLevelData> SelectedLevel;

	UPROPERTY()
	TObjectPtr<UTurboLevelEntryWidget> FirstEntry;
};