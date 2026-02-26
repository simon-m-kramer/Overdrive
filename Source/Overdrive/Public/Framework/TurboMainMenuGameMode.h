// Copyright Simon Kramer. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TurboMainMenuGameMode.generated.h"

class UTurboRootLayout;
class UTurboMainMenuWidget;

UCLASS()
class OVERDRIVE_API ATurboMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    UTurboRootLayout* GetRootLayout() const { return RootLayout; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UTurboRootLayout> RootLayoutClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UTurboMainMenuWidget> MainMenuWidgetClass;

private:
    UPROPERTY()
    TObjectPtr<UTurboRootLayout> RootLayout;
};