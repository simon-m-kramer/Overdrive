// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UTurboButtonBase::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (ButtonText && !ButtonLabel.IsEmpty())
    {
        ButtonText->SetText(ButtonLabel);
    }

}

void UTurboButtonBase::NativeOnCurrentTextStyleChanged()
{
    Super::NativeOnCurrentTextStyleChanged();
    // Common UI calls this when button state changes — 
    // good place to update visuals if you use CommonUI styles later
}

void UTurboButtonBase::NativeOnHovered()
{
    Super::NativeOnHovered();
}

void UTurboButtonBase::NativeOnUnhovered()
{
    Super::NativeOnUnhovered();
}

