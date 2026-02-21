// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboStandingsWidget.generated.h"

class UVerticalBox;
class UTurboStandingsEntryWidget;
class UTurboRaceManager;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboStandingsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox_Entries;

	UPROPERTY(EditAnywhere, Category = "Standings")
	TSubclassOf<UTurboStandingsEntryWidget> EntryWidgetClass;

private:
	TWeakObjectPtr<UTurboRaceManager> CachedRaceManager;
	TWeakObjectPtr<ATurboVehicle> CachedVehicle;

	UPROPERTY()
	TArray<TObjectPtr<UTurboStandingsEntryWidget>> EntryWidgets;

	void RebuildEntries(int32 Count);
};