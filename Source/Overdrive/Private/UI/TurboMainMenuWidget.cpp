// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboMainMenuWidget.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/TurboCreditsWidget.h"
#include "UI/TurboRootLayout.h"
#include "UI/TurboSettingsWidget.h"

void UTurboMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Start)
	{
		Btn_Start->OnClicked().AddUObject(this, &ThisClass::OnStartClicked);
	}
	if (Btn_Quit)
	{
		Btn_Quit->OnClicked().AddUObject(this, &ThisClass::OnQuitClicked);
	}
	if (Btn_Credits)
	{
		Btn_Credits->OnClicked().AddUObject(this, &ThisClass::OnCreditsClicked);
	}
	if (Btn_Settings)
	{
		Btn_Settings->OnClicked().AddUObject(this, &ThisClass::OnSettingsClicked);
	}
}

void UTurboMainMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void UTurboMainMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
}

void UTurboMainMenuWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(this, FName("L_ProvingGrounds"));
}

void UTurboMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void UTurboMainMenuWidget::OnCreditsClicked()
{
	if (UTurboRootLayout* Root = UTurboRootLayout::GetRootLayout(this))
	{
		Root->PushWidget(CreditsWidgetClass);
	}
}

void UTurboMainMenuWidget::OnSettingsClicked()
{
	if (UTurboRootLayout* Root = UTurboRootLayout::GetRootLayout(this))
	{
		Root->PushWidget(SettingsWidgetClass);
	}
}
