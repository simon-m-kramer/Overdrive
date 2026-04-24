// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboMainMenuWidget.generated.h"

class UCommonButtonBase;
class UTurboCreditsWidget;
class UTurboSettingsWidget;
class UTurboLevelSelectWidget;

UCLASS(Abstract)
class OVERDRIVE_API UTurboMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Start;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_LevelSelect;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Quit;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Credits;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Settings;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboLevelSelectWidget> LevelSelectWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboCreditsWidget> CreditsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboSettingsWidget> SettingsWidgetClass;

private:
	void OnStartClicked();
	void OnQuitClicked();
	void OnCreditsClicked();
	void OnSettingsClicked();
	void OnLevelSelectClicked();

};