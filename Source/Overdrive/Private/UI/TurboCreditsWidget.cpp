// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboCreditsWidget.h"
#include "CommonButtonBase.h"

void UTurboCreditsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &UTurboCreditsWidget::OnBackClicked);
    }
}

void UTurboCreditsWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UTurboCreditsWidget::OnBackClicked()
{
    DeactivateWidget();
}

UWidget* UTurboCreditsWidget::NativeGetDesiredFocusTarget() const
{
    return Btn_Back;
}