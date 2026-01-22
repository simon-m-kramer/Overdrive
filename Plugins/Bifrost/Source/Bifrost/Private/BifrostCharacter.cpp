// Copyright Simon Kramer. All Rights Reserved.


#include "BifrostCharacter.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"


ABifrostCharacter::ABifrostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABifrostCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ActionStack = NewObject<UBifrostActionStack>(this);
}

void ABifrostCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void ABifrostCharacter::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void ABifrostCharacter::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool ABifrostCharacter::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool ABifrostCharacter::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* ABifrostCharacter::GetCurrentAction() const
{
	if (ActionStack)
	{
		ActionStack->GetCurrentAction();
	}
	return nullptr;
}
