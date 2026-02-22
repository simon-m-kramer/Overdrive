// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboRaceResultWidget.h"
#include "Framework/TurboRaceManager.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"

void UTurboRaceResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked().AddUObject(this, &UTurboRaceResultWidget::OnMainMenuClicked);
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

void UTurboRaceResultWidget::SetResult(int32 Position, int32 TotalEntries, float BestLapTime)
{
	if (Txt_Position)
	{
		Txt_Position->SetText(FText::FromString(FString::Printf(TEXT("%d"), Position)));  // Txt_Position->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Position, TotalEntries)));
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
}

void UTurboRaceResultWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenu"));
}