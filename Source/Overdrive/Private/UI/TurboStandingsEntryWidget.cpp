// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboStandingsEntryWidget.h"
#include "Framework/TurboRaceManager.h"
#include "CommonTextBlock.h"

void UTurboStandingsEntryWidget::SetEntry(int32 Position, const FString& DriverName, bool bIsPlayer, float BestLapTime)
{
	if (Txt_Position)
	{
		Txt_Position->SetText(FText::FromString(FString::Printf(TEXT("%d"), Position)));
	}

	if (Txt_DriverName)
	{
		Txt_DriverName->SetText(FText::FromString(DriverName));
	}

	if (Txt_BestLapTime)
	{
		if (BestLapTime > 0.0f)
		{
			Txt_BestLapTime->SetText(FText::FromString(UTurboRaceManager::FormatLapTime(BestLapTime)));
		}
		else
		{
			Txt_BestLapTime->SetText(FText::FromString(TEXT("--:--.---")));
		}
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
