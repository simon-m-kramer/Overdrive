// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboLapCounterWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboGameMode.h"
#include "CommonTextBlock.h"

void UTurboLapCounterWidget::NativeConstruct()
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

	if (Txt_TotalLaps && CachedRaceManager.IsValid())
	{
		Txt_TotalLaps->SetText(FText::AsNumber(CachedRaceManager->TotalLaps));
	}
}

void UTurboLapCounterWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedRaceManager.IsValid() || !CachedVehicle.IsValid())
	{
		return;
	}

	if (Txt_CurrentLap)
	{
		const int32 Lap = FMath::Max(1, CachedRaceManager->GetCurrentLap(CachedVehicle.Get()));
		Txt_CurrentLap->SetText(FText::AsNumber(Lap));
	}
}