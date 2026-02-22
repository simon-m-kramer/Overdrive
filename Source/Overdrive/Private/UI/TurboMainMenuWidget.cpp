// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboMainMenuWidget.h"
#include "UI/TurboLevelSelectWidget.h"
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

	if (Btn_LevelSelect)
	{
		Btn_LevelSelect->OnClicked().AddUObject(this, &ThisClass::OnLevelSelectClicked);
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

	if (Btn_LevelSelect)
	{
		Btn_LevelSelect->OnClicked().RemoveAll(this);
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