// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboLevelSelectWidget.h"
#include "UI/TurboLevelSelectEntryWidget.h"
#include "CommonButtonBase.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Kismet/GameplayStatics.h"

void UTurboLevelSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Start)
	{
		Btn_Start->OnClicked().AddUObject(this, &UTurboLevelSelectWidget::OnStartClicked);
		Btn_Start->SetIsEnabled(false);
	}

	if (Btn_Back)
	{
		Btn_Back->OnClicked().AddUObject(this, &UTurboLevelSelectWidget::OnBackClicked);
	}

	if (!HBox_Levels || !EntryWidgetClass)
	{
		return;
	}

	// Clear out preview
	HBox_Levels->ClearChildren();

	// Create Level Previews
	for (const FTurboLevelEntry& Level : Levels)
	{
		UTurboLevelSelectEntryWidget* Entry = CreateWidget<UTurboLevelSelectEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		if (Entry)
		{
			Entry->SetEntry(Level);
			Entry->OnClicked().AddWeakLambda(this, [this, Entry]()
				{
					OnLevelSelected(Entry);
				});

			UHorizontalBoxSlot* HorizontalBoxSlot = HBox_Levels->AddChildToHorizontalBox(Entry);
			if (HorizontalBoxSlot)
			{
				HorizontalBoxSlot->SetPadding(FMargin(10.0f));
			}

			EntryWidgets.Add(Entry);
		}
	}

	if (!EntryWidgets.IsEmpty())
	{
		OnLevelSelected(EntryWidgets[0]);
	}
}

void UTurboLevelSelectWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UTurboLevelSelectWidget::OnLevelSelected(UTurboLevelSelectEntryWidget* Clicked)
{
	if (!Clicked)
	{
		return;
	}

	for (UTurboLevelSelectEntryWidget* Entry : EntryWidgets)
	{
		Entry->SetIsSelected(false);
	}

	Clicked->SetIsSelected(true);
	SelectedEntry = Clicked;

	if (Btn_Start)
	{
		Btn_Start->SetIsEnabled(true);
	}
}

void UTurboLevelSelectWidget::OnStartClicked()
{
	if (SelectedEntry)
	{
		UGameplayStatics::OpenLevel(GetWorld(), SelectedEntry->GetEntry().LevelName);
	}
}

void UTurboLevelSelectWidget::OnBackClicked()
{
	DeactivateWidget();
	RemoveFromParent();
}