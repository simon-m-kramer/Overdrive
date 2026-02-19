// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboMainMenuWidget.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UTurboMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (StartButton)
	{
		StartButton->OnClicked().AddUObject(this, &ThisClass::OnStartClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &ThisClass::OnQuitClicked);
	}
}

void UTurboMainMenuWidget::NativeOnDeactivated()
{
	if (StartButton)
	{
		StartButton->OnClicked().RemoveAll(this);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked().RemoveAll(this);
	}

	Super::NativeOnDeactivated();
}

void UTurboMainMenuWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(this, FName("L_ProvingGrounds"));
}

void UTurboMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		nullptr,
		EQuitPreference::Quit,
		true
	);
}