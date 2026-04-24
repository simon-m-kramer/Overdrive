// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboLevelSelectWidget.h"
#include "Components/WrapBox.h"
#include "CommonButtonBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TurboLevelData.h"
#include "UI/TurboLevelEntryWidget.h"

void UTurboLevelSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Play)
	{
		Btn_Play->OnClicked().AddUObject(this, &ThisClass::OnPlayClicked);
		Btn_Play->SetIsEnabled(false);
	}
	if (Btn_Back)
	{
		Btn_Back->OnClicked().AddUObject(this, &ThisClass::OnBackClicked);
	}

	PopulateLevelGrid();
}

void UTurboLevelSelectWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (!SelectedLevel && Levels.Num() > 0)
	{
		OnEntrySelected(Levels[0]);
	}
}

void UTurboLevelSelectWidget::PopulateLevelGrid()
{
	if (!LevelGrid || !EntryWidgetClass)
	{
		return;
	}

	LevelGrid->ClearChildren();
	FirstEntry = nullptr;

	for (UTurboLevelData* LevelData : Levels)
	{
		if (!LevelData)
		{
			continue;
		}

		UTurboLevelEntryWidget* Entry = CreateWidget<UTurboLevelEntryWidget>(this, EntryWidgetClass);
		if (!Entry)
		{
			continue;
		}

		Entry->SetLevelData(LevelData);
		Entry->OnLevelEntrySelected.AddDynamic(this, &ThisClass::OnEntrySelected);
		LevelGrid->AddChild(Entry);

		if (!FirstEntry)
		{
			FirstEntry = Entry;
		}
	}
}

UWidget* UTurboLevelSelectWidget::NativeGetDesiredFocusTarget() const
{
	return FirstEntry ? static_cast<UWidget*>(FirstEntry) : static_cast<UWidget*>(Btn_Back);
}

void UTurboLevelSelectWidget::OnEntrySelected(UTurboLevelData* InLevelData)
{
	SelectedLevel = InLevelData;

	if (Btn_Play)
	{
		Btn_Play->SetIsEnabled(SelectedLevel != nullptr);
	}
}

void UTurboLevelSelectWidget::OnPlayClicked()
{
	if (!SelectedLevel || SelectedLevel->Level.IsNull())
	{
		return;
	}

	if (SelectedLevel->Level.IsValid())
	{
		OnLevelLoadedForPlay();
		return;
	}

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(SelectedLevel->Level.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UTurboLevelSelectWidget::OnLevelLoadedForPlay));
}

void UTurboLevelSelectWidget::OnLevelLoadedForPlay()
{
	if (!SelectedLevel)
	{
		return;
	}

	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(SelectedLevel->Level.ToString()));
	UGameplayStatics::OpenLevel(this, LevelName);
}

void UTurboLevelSelectWidget::OnBackClicked()
{
	DeactivateWidget();
}