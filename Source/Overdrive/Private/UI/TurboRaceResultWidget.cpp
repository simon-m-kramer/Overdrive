// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboRaceResultWidget.h"
#include "UI/TurboStandingsEntryWidget.h"
#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UTurboRaceResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked().AddUObject(this, &ThisClass::OnMainMenuClicked);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void UTurboRaceResultWidget::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}

	Super::NativeDestruct();
}

void UTurboRaceResultWidget::SetResult(UTurboRaceManager* RaceManager, ATurboVehicle* PlayerVehicle)
{
	if (!RaceManager || !PlayerVehicle)
	{
		return;
	}

	if (Txt_Position)
	{
		Txt_Position->SetText(FText::FromString(FString::Printf(TEXT("P%d/%d"), RaceManager->GetPlacement(PlayerVehicle), RaceManager->GetEntryCount())));
	}

	if (Txt_BestLapTime)
	{
		const float BestTime = RaceManager->GetBestLapTime(PlayerVehicle);
		if (BestTime > 0.0f)
		{
			Txt_BestLapTime->SetText(FText::FromString(UTurboRaceManager::FormatLapTime(BestTime)));
		}
		else
		{
			Txt_BestLapTime->SetText(FText::FromString(TEXT("--:--.---")));
		}
	}

	if (!VBox_Results || !EntryWidgetClass)
	{
		return;
	}

	VBox_Results->ClearChildren();

	const TArray<FRaceEntry>& Standings = RaceManager->GetStandings();

	for (const FRaceEntry& Standing : Standings)
	{
		if (!Standing.Vehicle.IsValid())
		{
			continue;
		}

		UTurboStandingsEntryWidget* Entry = CreateWidget<UTurboStandingsEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (Entry)
		{
			Entry->SetEntry(Standing.Placement, Standing.Vehicle->VehicleName, Standing.Vehicle == PlayerVehicle, Standing.BestLapTime);
			VBox_Results->AddChildToVerticalBox(Entry);
		}
	}
}

void UTurboRaceResultWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("L_MainMenu_Rainy"));
}