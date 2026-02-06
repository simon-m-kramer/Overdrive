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
	// =========================================================================
	// ACTION MANAGEMENT
	// =========================================================================

	virtual void PushAction(UBifrostAction* NewAction) override;
	virtual void RemoveAction(UBifrostAction* InAction) override;
	virtual void UpdateActions(float DeltaTime) override;

	/** Evaluate the priority list and push the first valid action */
	void EvaluateActions(ATurboAIController* Controller);

	// =========================================================================
	// ACTION TAGS
	// =========================================================================

	/** Check if an action with the given tag would be blocked by the current stack */
	UFUNCTION(BlueprintPure, Category = "Action Tags")
	bool IsActionBlocked(FGameplayTag ActionTag) const;

	/** Get all active action tags on the stack */
	UFUNCTION(BlueprintPure, Category = "Action Tags")
	const FGameplayTagContainer& GetActiveActionTags() const { return ActiveActionTags; }

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	/** Actions to evaluate each tick, in priority order (first = highest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
	TArray<TSubclassOf<UTurboActionBase>> ActionPriorityList;

	/** Actions disabled by personality — these tags will always be blocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
	FGameplayTagContainer DisabledActions;

	// =========================================================================
	// DEBUG
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowEvaluationDebug = false;

private:
	void RebuildActiveActionTags();

	FGameplayTagContainer ActiveActionTags;
};
