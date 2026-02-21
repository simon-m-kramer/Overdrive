// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboLapTimerWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboGameMode.h"
#include "CommonTextBlock.h"

void UTurboLapTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(GetWorld()->GetAuthGameMode()))
	{
		CachedRaceManager = GameMode->GetRaceManager();
	}

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		CachedVehicle = Cast<ATurboVehicle>(Pawn);
	}
}

void UTurboLapTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedRaceManager.IsValid() || !CachedVehicle.IsValid())
	{
		return;
	}

	if (Txt_LapTime)
	{
		const float LapTime = CachedRaceManager->GetCurrentLapTime(CachedVehicle.Get());
		Txt_LapTime->SetText(FText::FromString(UTurboRaceManager::FormatLapTime(LapTime)));
	}

	if (Txt_BestLapTime)
	{
		const float BestTime = CachedRaceManager->GetBestLapTime(CachedVehicle.Get());
		if (BestTime > 0.0f)
		{
			Txt_BestLapTime->SetText(FText::FromString(UTurboRaceManager::FormatLapTime(BestTime)));
		}
		else
		{
			Txt_BestLapTime->SetText(FText::FromString(TEXT("--:--.---")));
		}
	}
}