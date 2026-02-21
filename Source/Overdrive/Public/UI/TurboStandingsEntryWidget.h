// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboStandingsEntryWidget.generated.h"

class UCommonTextBlock;

UCLASS()
class OVERDRIVE_API UTurboStandingsEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetEntry(int32 Position, const FString& DriverName, bool bIsPlayer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_Position;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Txt_DriverName;
};
