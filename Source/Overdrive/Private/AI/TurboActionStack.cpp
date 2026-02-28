// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboActionStack.h"
#include "AI/TurboActionBase.h"
#include "AI/TurboAIController.h"
#include "BifrostAction.h"


void UTurboActionStack::SetActionInstances(const TArray<UTurboActionBase*>& InActions)
{
	ActionInstances = InActions;
}

void UTurboActionStack::EvaluateActions(const FTurboDecisionContext& Context)
{
	for (UTurboActionBase* Action : ActionInstances)
	{
		if (Action && !IsActionBlocked(Action->ActionTag) && Action->CanActivate(Context))
		{
			PushAction(Action);
			return;
		}
	}
}

bool UTurboActionStack::IsActionBlocked(FGameplayTag ActionTag) const
{
    if (!ActionTag.IsValid()) return false;

    for (UBifrostAction* Action : GetActions())
    {
        UTurboActionBase* TurboAction = Cast<UTurboActionBase>(Action);
        if (TurboAction && TurboAction->BlocksTags.HasTag(ActionTag))
        {
            return true;
        }
    }

    return false;
}

