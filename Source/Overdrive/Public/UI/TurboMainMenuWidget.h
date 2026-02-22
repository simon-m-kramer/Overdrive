// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboMainMenuWidget.generated.h"

class UCommonButtonBase;
class UTurboLevelSelectWidget;

UCLASS(Abstract)
class OVERDRIVE_API UTurboMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Start;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_LevelSelect;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> Btn_Quit;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UTurboLevelSelectWidget> LevelSelectClass;

private:
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnLevelSelectClicked();

	UFUNCTION()
	void OnQuitClicked();
};