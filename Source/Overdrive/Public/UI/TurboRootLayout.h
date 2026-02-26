// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TurboRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

UCLASS()
class OVERDRIVE_API UTurboRootLayout : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void PushWidget(TSubclassOf<UCommonActivatableWidget> WidgetClass);

    UFUNCTION(BlueprintCallable)
    void PopWidget();

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static UTurboRootLayout* GetRootLayout(const UObject* WorldContextObject);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> MenuStack;

};