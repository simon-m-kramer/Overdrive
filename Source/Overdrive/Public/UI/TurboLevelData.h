// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TurboLevelData.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class OVERDRIVE_API UTurboLevelData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Level", meta = (AllowedClasses = "/Script/Engine.Texture2D"))
	TSoftObjectPtr<UTexture2D> PreviewImage;

	UPROPERTY(EditDefaultsOnly, Category = "Level", meta = (AllowedClasses = "/Script/Engine.World"))
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditDefaultsOnly, Category = "Level", meta = (MultiLine = true))
	FText Description;
};