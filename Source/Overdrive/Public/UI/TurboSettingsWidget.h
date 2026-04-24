// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TurboSettingsWidget.generated.h"

class UCommonButtonBase;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboSettingsWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    virtual UWidget* NativeGetDesiredFocusTarget() const override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;

private:
    UFUNCTION()
    void OnBackClicked();
};
