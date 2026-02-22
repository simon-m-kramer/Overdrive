// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "UI/TurboLevelEntry.h"
#include "TurboLevelSelectEntryWidget.generated.h"

class UCommonTextBlock;
class UImage;

UCLASS()
class OVERDRIVE_API UTurboLevelSelectEntryWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	void SetEntry(const FTurboLevelEntry& InEntry);
	const FTurboLevelEntry& GetEntry() const { return Entry; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_LevelName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Preview;

private:
	FTurboLevelEntry Entry;
};