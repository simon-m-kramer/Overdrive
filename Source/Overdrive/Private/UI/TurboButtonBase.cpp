// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboButtonBase.h"
#include "CommonTextBlock.h"

void UTurboButtonBase::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (ButtonText && !ButtonLabel.IsEmpty())
    {
        ButtonText->SetText(ButtonLabel);
    }
}