// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboPauseMenuWidget.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"

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

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void UTurboPauseMenuWidget::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}

	Super::NativeDestruct();
}

void UTurboPauseMenuWidget::OnResumeClicked()
{
	RemoveFromParent();
}

void UTurboPauseMenuWidget::OnMainMenuClicked()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(GetWorld(), FName("L_MainMenu_Rainy"));
}