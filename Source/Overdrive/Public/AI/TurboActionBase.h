// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BifrostAction.h"
#include "GameplayTagContainer.h"
#include "TurboActionBase.generated.h"

class ATurboAIController;
/**
 * 
 */
UCLASS(Abstract)
class OVERDRIVE_API UTurboActionBase : public UBifrostAction
{
	GENERATED_BODY()
	
public:
    /** Check if this action can activate given current context. Called on CDO. */
    virtual bool CanActivate(const ATurboAIController* Controller) const { return false; }

    /** Identifies this action type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTag ActionTag;

    /** This action cannot be pushed if any of these tags are active on the stack */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTagContainer BlockedByTags;

    /** While this action is on the stack, it blocks actions with these tags from being pushed */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Tags")
    FGameplayTagContainer BlocksTags;

};
