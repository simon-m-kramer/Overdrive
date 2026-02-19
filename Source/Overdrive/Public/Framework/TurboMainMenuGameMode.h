// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TurboMainMenuGameMode.generated.h"

class UTurboMainMenuWidget;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

protected:
	// Set this in the Blueprint child class to your WBP_MainMenu
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTurboMainMenuWidget> MainMenuWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UTurboMainMenuWidget> MainMenuInstance;

};
