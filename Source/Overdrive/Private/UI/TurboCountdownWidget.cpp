// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboCountdownWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboGameMode.h"
#include "CommonTextBlock.h"

void UTurboCountdownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UTurboRaceManager* RaceManager = GameMode->GetRaceManager())
		{
			RaceManager->OnCountdownUpdated.AddDynamic(this, &UTurboCountdownWidget::OnCountdownUpdated);
			RaceManager->OnRaceStarted.AddDynamic(this, &UTurboCountdownWidget::OnRaceStarted);
		}
	}
}

void UTurboCountdownWidget::NativeDestruct()
{
	if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UTurboRaceManager* RaceManager = GameMode->GetRaceManager())
		{
			RaceManager->OnCountdownUpdated.RemoveDynamic(this, &UTurboCountdownWidget::OnCountdownUpdated);
			RaceManager->OnRaceStarted.RemoveDynamic(this, &UTurboCountdownWidget::OnRaceStarted);
		}
	}

	Super::NativeDestruct();
}

void UTurboCountdownWidget::OnCountdownUpdated(int32 Count)
{
	if (Txt_Countdown)
	{
		if (Count > 0)
		{
			Txt_Countdown->SetText(FText::AsNumber(Count));
		}
		else
		{
			Txt_Countdown->SetText(FText::FromString(TEXT("GO!")));
		}
	}
}

void UTurboCountdownWidget::OnRaceStarted()
{
	RemoveFromParent();
}
