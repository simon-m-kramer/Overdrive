// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurboLevelEntry.generated.h"

USTRUCT(BlueprintType)
struct FTurboLevelEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> PreviewImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName LevelName;
};