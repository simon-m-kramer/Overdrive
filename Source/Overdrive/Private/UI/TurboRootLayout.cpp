// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboRootLayout.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Framework/TurboMainMenuGameMode.h"

void UTurboRootLayout::PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
    if (MenuStack && WidgetClass)
    {
        MenuStack->AddWidget(WidgetClass);
    }
}

void UTurboRootLayout::PopWidget()
{
    if (MenuStack)
    {
        if (UCommonActivatableWidget* ActiveWidget = MenuStack->GetActiveWidget())
        {
            ActiveWidget->DeactivateWidget();
        }
    }
}

UTurboRootLayout* UTurboRootLayout::GetRootLayout(const UObject* WorldContextObject)
{
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (ATurboMainMenuGameMode* GM = Cast<ATurboMainMenuGameMode>(World->GetAuthGameMode()))
        {
            return GM->GetRootLayout();
        }
    }
    return nullptr;
}