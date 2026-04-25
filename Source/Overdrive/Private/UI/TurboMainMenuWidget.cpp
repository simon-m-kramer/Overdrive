// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboMainMenuWidget.h"
#include "CommonButtonBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/TurboCreditsWidget.h"
#include "UI/TurboRootLayout.h"
#include "UI/TurboSettingsWidget.h"
#include "Components/TextBlock.h"
#include "UI/TurboLevelData.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

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
	if (Btn_CyclePrev)
	{
		Btn_CyclePrev->OnClicked().AddUObject(this, &UTurboMainMenuWidget::OnCyclePrevClicked);
	}
	if (Btn_CycleNext)
	{
		Btn_CycleNext->OnClicked().AddUObject(this, &UTurboMainMenuWidget::OnCycleNextClicked);
	}
	UpdateLevelDisplay();
}

UWidget* UTurboMainMenuWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_Start;
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
	/*
	UGameplayStatics::OpenLevel(this, FName("L_ProvingGrounds"));
	*/

	if (!Levels.IsValidIndex(CurrentLevelIndex)) return;
	UTurboLevelData* Current = Levels[CurrentLevelIndex];
	if (!Current || Current->Level.IsNull()) return;

	if (Current->Level.IsValid())
	{
		const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Current->Level.ToString()));
		UGameplayStatics::OpenLevel(this, LevelName);
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(
		Current->Level.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda([this]()
			{
				if (!Levels.IsValidIndex(CurrentLevelIndex)) return;
				UTurboLevelData* Loaded = Levels[CurrentLevelIndex];
				if (!Loaded) return;
				const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Loaded->Level.ToString()));
				UGameplayStatics::OpenLevel(this, LevelName);
			}));
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

void UTurboMainMenuWidget::OnCyclePrevClicked()
{
	if (Levels.Num() == 0) return;
	CurrentLevelIndex = (CurrentLevelIndex - 1 + Levels.Num()) % Levels.Num();
	UpdateLevelDisplay();
}

void UTurboMainMenuWidget::OnCycleNextClicked()
{
	if (Levels.Num() == 0) return;
	CurrentLevelIndex = (CurrentLevelIndex + 1) % Levels.Num();
	UpdateLevelDisplay();
}

void UTurboMainMenuWidget::UpdateLevelDisplay()
{
	if (!Levels.IsValidIndex(CurrentLevelIndex)) return;

	UTurboLevelData* Current = Levels[CurrentLevelIndex];
	if (!Current) return;

	if (Txt_LevelName)
	{
		Txt_LevelName->SetText(Current->DisplayName);
	}

	if (Txt_LevelDescription)
	{
		Txt_LevelDescription->SetText(Current->Description);
	}
}




