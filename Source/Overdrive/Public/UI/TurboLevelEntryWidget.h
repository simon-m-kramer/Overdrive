// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "TurboLevelEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UTurboLevelData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelEntrySelected, UTurboLevelData*, LevelData);

UCLASS(Abstract)
class OVERDRIVE_API UTurboLevelEntryWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	void SetLevelData(UTurboLevelData* InLevelData);

	UTurboLevelData* GetLevelData() const { return LevelData; }

	FOnLevelEntrySelected OnLevelEntrySelected;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Preview;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_LevelName;

private:
	UPROPERTY()
	TObjectPtr<UTurboLevelData> LevelData;

	void OnPreviewImageLoaded();

	void HandleButtonClicked();
};