// Copyright Simon Kramer. All Rights Reserved.

#include "UI/TurboLevelEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "UI/TurboLevelData.h"

void UTurboLevelEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	OnClicked().AddUObject(this, &ThisClass::HandleButtonClicked);
}

void UTurboLevelEntryWidget::SetLevelData(UTurboLevelData* InLevelData)
{
	LevelData = InLevelData;

	if (!LevelData)
	{
		return;
	}

	if (Txt_LevelName)
	{
		Txt_LevelName->SetText(LevelData->DisplayName);
	}

	if (Img_Preview && !LevelData->PreviewImage.IsNull())
	{
		if (LevelData->PreviewImage.IsValid())
		{
			Img_Preview->SetBrushFromTexture(LevelData->PreviewImage.Get());
		}
		else
		{
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			Streamable.RequestAsyncLoad(LevelData->PreviewImage.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UTurboLevelEntryWidget::OnPreviewImageLoaded));
		}
	}
}

void UTurboLevelEntryWidget::OnPreviewImageLoaded()
{
	if (Img_Preview && LevelData && LevelData->PreviewImage.IsValid())
	{
		Img_Preview->SetBrushFromTexture(LevelData->PreviewImage.Get());
	}
}

void UTurboLevelEntryWidget::HandleButtonClicked()
{
	OnLevelEntrySelected.Broadcast(LevelData);
}