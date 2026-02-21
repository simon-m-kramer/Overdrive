// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboStandingsEntryWidget.h"
#include "CommonTextBlock.h"

void UTurboStandingsEntryWidget::SetEntry(int32 Position, const FString& DriverName, bool bIsPlayer)
{
	if (Txt_Position)
	{
		Txt_Position->SetText(FText::FromString(FString::Printf(TEXT("%d"), Position)));
	}

	if (Txt_DriverName)
	{
		Txt_DriverName->SetText(FText::FromString(DriverName));
	}

	// Highlight the player's row
	if (bIsPlayer)
	{
		SetColorAndOpacity(FLinearColor(1.0f, 0.015f, 0.0f));
	}
	else
	{
		SetColorAndOpacity(FLinearColor::White);
	}
}