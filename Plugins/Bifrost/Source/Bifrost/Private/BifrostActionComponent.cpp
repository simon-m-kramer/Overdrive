// Copyright Simon Kramer. All Rights Reserved.


#include "BifrostActionComponent.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"

UBifrostActionComponent::UBifrostActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBifrostActionComponent::BeginPlay()
{
	Super::BeginPlay();

	ActionStack = NewObject<UBifrostActionStack>(this);
}

void UBifrostActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void UBifrostActionComponent::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void UBifrostActionComponent::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool UBifrostActionComponent::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool UBifrostActionComponent::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* UBifrostActionComponent::GetCurrentAction() const
{
	if (ActionStack)
	{
		ActionStack->GetCurrentAction();
	}
	return nullptr;
}