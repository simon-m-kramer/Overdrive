// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboLevelSelectEntryWidget.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UTurboLevelSelectEntryWidget::SetEntry(const FTurboLevelEntry& InEntry)
{
	Entry = InEntry;

	if (Txt_LevelName)
	{
		Txt_LevelName->SetText(Entry.DisplayName);
	}

	if (Img_Preview)
	{
		UTexture2D* Texture = Entry.PreviewImage.LoadSynchronous();
		if (Texture)
		{
			Img_Preview->SetBrushFromTexture(Texture);
		}
	}
}