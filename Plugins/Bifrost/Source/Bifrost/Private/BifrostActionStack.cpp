// Copyright Simon Kramer. All Rights Reserved.


#include "BifrostActionStack.h"
#include "BifrostAction.h"

void UBifrostActionStack::UpdateActions(float DeltaTime)
{
    if (IsEmpty())
    {
        return;
    }

    while (!CurrentAction && ActionStack.Num() > 0)
    {
        CurrentAction = ActionStack[0];

        bool bFirstTime = !FirstTimeActions.Contains(CurrentAction);  // Determine if this is the first time this action is executing
        FirstTimeActions.Add(CurrentAction);
        CurrentAction->Start(bFirstTime);

        if (CurrentAction)
        {
            if (ActionStack.Num() > 0 && CurrentAction != ActionStack[0])
            {
                CurrentAction = nullptr;
                UpdateActions(DeltaTime);
                return;
            }
        }
    }

    if (CurrentAction)
    {
        // Update Phase
        CurrentAction->Update(DeltaTime);

        // Cleanup Phase
        if (ActionStack.Num() > 0 && CurrentAction == ActionStack[0])
        {
            if (CurrentAction->IsDone())
            {
                ActionStack.RemoveAt(0);
                CurrentAction->End();
                FirstTimeActions.Remove(CurrentAction);
                CurrentAction = nullptr;
            }
        }
        else
        {
            CurrentAction = nullptr;
        }
    }
}

void UBifrostActionStack::PushAction(UBifrostAction* NewAction)
{
    if (!NewAction)
    {
        return;
    }

    // Remove if already in list
    ActionStack.Remove(NewAction);

    // Insert at front
    ActionStack.Insert(NewAction, 0);

    // Reset current action if different
    if (CurrentAction && CurrentAction != NewAction)
    {
        CurrentAction = nullptr;
    }
}

void UBifrostActionStack::RemoveAction(UBifrostAction* InAction)
{
    if (Contains(InAction))
    {
        if (CurrentAction == InAction)
        {
            InAction->End();
            CurrentAction = nullptr;
        }
        ActionStack.Remove(InAction);
    }
}

bool UBifrostActionStack::Contains(UBifrostAction* InAction)
{
    return InAction != nullptr && ActionStack.Contains(InAction);
}




