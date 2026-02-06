// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboActionStack.h"
#include "AI/TurboActionBase.h"
#include "AI/TurboAIController.h"
#include "BifrostAction.h"

void UTurboActionStack::PushAction(UBifrostAction* NewAction)
{
	Super::PushAction(NewAction);
	RebuildActiveActionTags();
}

void UTurboActionStack::RemoveAction(UBifrostAction* InAction)
{
	Super::RemoveAction(InAction);
	RebuildActiveActionTags();
}

void UTurboActionStack::UpdateActions(float DeltaTime)
{
	//EvaluateActions();    // decide
	Super::UpdateActions(DeltaTime);  // execute
}

void UTurboActionStack::EvaluateActions(ATurboAIController* Controller)
{
	if (!Controller)
	{
		return;
	}

	for (TSubclassOf<UTurboActionBase> ActionClass : ActionPriorityList)
	{
		if (!ActionClass)
		{
			continue;
		}

		UTurboActionBase* CDO = ActionClass->GetDefaultObject<UTurboActionBase>();
		if (!CDO)
		{
			continue;
		}

		if (IsActionBlocked(CDO->ActionTag))
		{
			continue;
		}

		if (!CDO->CanActivate(Controller))
		{
			continue;
		}

		UTurboActionBase* NewAction = NewObject<UTurboActionBase>(this, ActionClass);
		PushAction(NewAction);

		if (bShowEvaluationDebug)
		{
			GEngine->AddOnScreenDebugMessage(40, 2.0f, FColor::Green,
				FString::Printf(TEXT("ACTION PUSHED: %s"), *CDO->ActionName));
		}

		// Only push one action per evaluation
		return;
	}
}

bool UTurboActionStack::IsActionBlocked(FGameplayTag ActionTag) const
{
	if (!ActionTag.IsValid())
	{
		return false;
	}

	// Disabled by personality
	if (DisabledActions.HasTag(ActionTag))
	{
		return true;
	}

	// Check if any active action blocks this tag
	UBifrostAction* Current = GetCurrentAction();
	UTurboActionBase* CurrentTurbo = Cast<UTurboActionBase>(Current);
	if (CurrentTurbo && CurrentTurbo->BlocksTags.HasTag(ActionTag))
	{
		return true;
	}

	const TArray<UBifrostAction*>& Actions = GetActions();
	for (UBifrostAction* Action : Actions)
	{
		UTurboActionBase* TurboAction = Cast<UTurboActionBase>(Action);
		if (TurboAction && TurboAction->BlocksTags.HasTag(ActionTag))
		{
			return true;
		}
	}

	return false;
}

void UTurboActionStack::RebuildActiveActionTags()
{
	ActiveActionTags.Reset();

	UTurboActionBase* Current = Cast<UTurboActionBase>(GetCurrentAction());
	if (Current && Current->ActionTag.IsValid())
	{
		ActiveActionTags.AddTag(Current->ActionTag);
	}

	const TArray<UBifrostAction*>& Actions = GetActions();
	for (UBifrostAction* Action : Actions)
	{
		UTurboActionBase* TurboAction = Cast<UTurboActionBase>(Action);
		if (TurboAction && TurboAction->ActionTag.IsValid())
		{
			ActiveActionTags.AddTag(TurboAction->ActionTag);
		}
	}
}