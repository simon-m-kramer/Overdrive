// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BifrostActionStack.h"
#include "GameplayTagContainer.h"
#include "TurboActionStack.generated.h"

class ATurboAIController;
class UTurboActionBase;
struct FTurboDecisionContext;

UCLASS()
class OVERDRIVE_API UTurboActionStack : public UBifrostActionStack
{
	GENERATED_BODY()

public:
	void SetActionInstances(const TArray<UTurboActionBase*>& InActions);

	void EvaluateActions(const FTurboDecisionContext& Context);

private:
	bool IsActionBlocked(FGameplayTag ActionTag) const;

	UPROPERTY()
	TArray<TObjectPtr<UTurboActionBase>> ActionInstances;
};
