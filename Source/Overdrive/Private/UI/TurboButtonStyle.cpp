// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboButtonStyle.h"

UTurboButtonStyle::UTurboButtonStyle()
{
    // This is the background of a button
	NormalBase.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
    NormalBase.DrawAs = ESlateBrushDrawType::Box;
    NormalBase.Margin = FMargin(0.5f);
    NormalHovered.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
    NormalHovered.DrawAs = ESlateBrushDrawType::Box;
    NormalHovered.Margin = FMargin(0.5f);
    NormalPressed.TintColor = FSlateColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
    NormalPressed.DrawAs = ESlateBrushDrawType::Box;
    NormalPressed.Margin = FMargin(0.5f);
}
