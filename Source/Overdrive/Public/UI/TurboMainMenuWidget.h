// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboMainMenuWidget.generated.h"

class UCommonButtonBase;
class UTurboLevelSelectWidget;
class UTurboCreditsWidget;
class UTurboSettingsWidget;

UCLASS(Abstract)
class OVERDRIVE_API UTurboMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

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

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UTurboLevelSelectWidget> LevelSelectClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboCreditsWidget> CreditsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboSettingsWidget> SettingsWidgetClass;

private:
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnLevelSelectClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnCreditsClicked();

	UFUNCTION()
	void OnSettingsClicked();

};