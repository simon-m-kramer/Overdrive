// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboCompassWidget.generated.h"

class UImage;
class ATurboVehicle;
class UMaterialInstanceDynamic;

UCLASS()
class OVERDRIVE_API UTurboCompassWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Compass;

	/** The UI material with an "Offset" scalar parameter */
	UPROPERTY(EditAnywhere, Category = "Compass")
	TObjectPtr<UMaterialInterface> CompassMaterial;

private:
	TWeakObjectPtr<ATurboVehicle> CachedVehicle;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CompassMID;
};