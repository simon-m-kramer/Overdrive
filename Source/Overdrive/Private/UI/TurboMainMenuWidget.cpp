// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboMainMenuWidget.h"
#include "UI/TurboLevelSelectWidget.h"
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
	if (Btn_LevelSelect)
	{
		Btn_LevelSelect->OnClicked().AddUObject(this, &ThisClass::OnLevelSelectClicked);
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

void UTurboMainMenuWidget::OnLevelSelectClicked()
{
	if (!LevelSelectClass)
	{
		return;
	}

	UTurboLevelSelectWidget* LevelSelect = CreateWidget<UTurboLevelSelectWidget>(GetOwningPlayer(), LevelSelectClass);
	if (LevelSelect)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		LevelSelect->AddToViewport(100);
		LevelSelect->ActivateWidget();
		LevelSelect->OnDeactivated().AddWeakLambda(this, [this]()
			{
				SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			});
	}
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
