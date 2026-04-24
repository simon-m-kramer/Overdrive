// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboSettingsWidget.h"
#include "CommonButtonBase.h"

void UTurboSettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &UTurboSettingsWidget::OnBackClicked);
    }
}

void UTurboSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UTurboSettingsWidget::OnBackClicked()
{
    DeactivateWidget();
}

UWidget* UTurboSettingsWidget::NativeGetDesiredFocusTarget() const
{
    return Btn_Back;
}