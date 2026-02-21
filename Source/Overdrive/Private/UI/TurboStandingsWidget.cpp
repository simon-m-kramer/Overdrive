// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboStandingsWidget.h"
#include "UI/TurboStandingsEntryWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboGameMode.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UTurboStandingsWidget::NativeConstruct()
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

	if (CachedRaceManager.IsValid())
	{
		RebuildEntries(CachedRaceManager->GetEntryCount());
	}
}

void UTurboStandingsWidget::RebuildEntries(int32 Count)
{
	if (!VBox_Entries || !EntryWidgetClass)
	{
		return;
	}

	VBox_Entries->ClearChildren();
	EntryWidgets.Empty();

	for (int32 i = 0; i < Count; i++)
	{
		UTurboStandingsEntryWidget* Entry = CreateWidget<UTurboStandingsEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (Entry)
		{
			VBox_Entries->AddChildToVerticalBox(Entry);
			EntryWidgets.Add(Entry);
		}
	}
}

void UTurboStandingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedRaceManager.IsValid())
	{
		return;
	}

	const TArray<FRaceEntry>& Standings = CachedRaceManager->GetStandings();

	for (int32 i = 0; i < Standings.Num() && i < EntryWidgets.Num(); i++)
	{
		if (!Standings[i].Vehicle.IsValid() || !EntryWidgets[i])
		{
			continue;
		}

		const bool bIsPlayer = (Standings[i].Vehicle == CachedVehicle);
		EntryWidgets[i]->SetEntry(Standings[i].Placement, Standings[i].Vehicle->VehicleName, bIsPlayer);
	}
}