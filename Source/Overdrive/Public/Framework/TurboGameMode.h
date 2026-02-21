// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TurboGameMode.generated.h"

class UTurboRaceManager;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboGameMode : public AGameModeBase
{
	GENERATED_BODY()
	

public:
	ATurboGameMode();

	UFUNCTION(BlueprintPure, Category = "Race")
	UTurboRaceManager* GetRaceManager() const { return RaceManager; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race")
	TObjectPtr<UTurboRaceManager> RaceManager;
};
