// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "TurboButtonBase.generated.h"


class UCommonTextBlock;

/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button")
	FText ButtonLabel;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> ButtonText;

	virtual void SynchronizeProperties() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;

private:


};
