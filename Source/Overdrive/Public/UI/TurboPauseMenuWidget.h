// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboPauseMenuWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class OVERDRIVE_API UTurboPauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()


public:
	void ShowMenu();
	void HideMenu();
	bool IsMenuVisible() const { return bIsMenuVisible; }


protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Btn_MainMenu;


private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	bool bIsMenuVisible = false;
};
