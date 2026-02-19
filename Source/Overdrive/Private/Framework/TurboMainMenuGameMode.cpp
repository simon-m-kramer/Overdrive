// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboMainMenuGameMode.h"
#include "UI/TurboMainMenuWidget.h"

void ATurboMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (MainMenuWidgetClass)
	{
		MainMenuInstance = CreateWidget<UTurboMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuInstance)
		{
			MainMenuInstance->AddToViewport();
			MainMenuInstance->ActivateWidget();

			// Let Common UI handle input routing — just show the cursor
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->bShowMouseCursor = true;
			}
		}
	}
}
