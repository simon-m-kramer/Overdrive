// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BifrostAction.h"
#include "GameplayTagContainer.h"
#include "TurboActionBase.generated.h"

struct FTurboDecisionContext;
/**
 * 
 */
UCLASS(Abstract)
class OVERDRIVE_API UTurboActionBase : public UBifrostAction
{
	GENERATED_BODY()
	
public:
    virtual bool CanActivate(const FTurboDecisionContext& Context) const { return false; }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTag ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTagContainer BlocksTags;

};
