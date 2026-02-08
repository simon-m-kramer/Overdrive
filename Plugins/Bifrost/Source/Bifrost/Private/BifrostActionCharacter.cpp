// Copyright Simon Kramer. All Rights Reserved.


#include "BifrostActionCharacter.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"


ABifrostActionCharacter::ABifrostActionCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABifrostActionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ActionStack = NewObject<UBifrostActionStack>(this);
}

void ABifrostActionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void ABifrostActionCharacter::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void ABifrostActionCharacter::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool ABifrostActionCharacter::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool ABifrostActionCharacter::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* ABifrostActionCharacter::GetCurrentAction() const
{
	if (ActionStack)
	{
		return ActionStack->GetCurrentAction();
	}
	return nullptr;
}

const TArray<UBifrostAction*>& ABifrostActionCharacter::GetActions() const
{
	static TArray<UBifrostAction*> EmptyArray;
	if (ActionStack)
	{
		return ActionStack->GetActions();
	}
	return EmptyArray;
}

