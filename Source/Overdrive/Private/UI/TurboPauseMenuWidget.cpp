// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboPauseMenuWidget.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TurboPlayerController.h"

void UTurboPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked().AddUObject(this, &UTurboPauseMenuWidget::OnResumeClicked);
	}

	if (Btn_MainMenu)
	{
		Btn_MainMenu->OnClicked().AddUObject(this, &UTurboPauseMenuWidget::OnMainMenuClicked);
	}
}

void UTurboPauseMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UTurboPauseMenuWidget::OnResumeClicked()
{
	if (ATurboPlayerController* PC = Cast<ATurboPlayerController>(GetOwningPlayer()))
	{
		PC->TogglePauseMenu();
	}
}

void UTurboPauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), FName("L_MainMenu_Rainy"));
}

void UTurboPauseMenuWidget::ShowMenu()
{
	ActivateWidget();  // Call base class
	SetVisibility(ESlateVisibility::Visible);
	bIsMenuVisible = true;
}

void UTurboPauseMenuWidget::HideMenu()
{
	DeactivateWidget();  // Call base class
	SetVisibility(ESlateVisibility::Collapsed);
	bIsMenuVisible = false;
}