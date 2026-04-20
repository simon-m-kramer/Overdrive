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

    /** This action's identifier tag */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTag ActionTag;

    /** Actions with these tags are blocked by this action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTagContainer BlocksTags;

};
