// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UTurboButtonBase::SetButtonText(FText InText)
{
    ButtonLabel = InText;
    if (ButtonText)
    {
        ButtonText->SetText(InText);
    }
}

void UTurboButtonBase::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    SetButtonText(ButtonLabel);
}

void UTurboButtonBase::NativeOnCurrentTextStyleChanged()
{
    Super::NativeOnCurrentTextStyleChanged();

    // This links the text style to the ButtonText
    if (ButtonText)
    {
        TSubclassOf<UCommonTextStyle> CurrentStyle = GetCurrentTextStyleClass();

        if (CurrentStyle)
        {
            ButtonText->SetStyle(CurrentStyle);
        }
    }
}

void UTurboButtonBase::NativeOnHovered()
{
    Super::NativeOnHovered();
}

void UTurboButtonBase::NativeOnUnhovered()
{
    Super::NativeOnUnhovered();
}

