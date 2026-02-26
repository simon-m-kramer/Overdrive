// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboMainMenuGameMode.h"
#include "UI/TurboRootLayout.h"
#include "UI/TurboMainMenuWidget.h"

void ATurboMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (RootLayoutClass && PC)
    {
        RootLayout = CreateWidget<UTurboRootLayout>(PC, RootLayoutClass);
        RootLayout->AddToViewport(100);
        RootLayout->PushWidget(MainMenuWidgetClass);

        PC->bShowMouseCursor = true;

        UE_LOG(LogTemp, Warning, TEXT("RootLayout created: %s"), *GetNameSafe(RootLayout));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Missing RootLayoutClass or PC"));
    }
}

