// Copyright Simon Kramer. All Rights Reserved.


#include "BifrostComponent.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"

UBifrostComponent::UBifrostComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBifrostComponent::BeginPlay()
{
	Super::BeginPlay();

	ActionStack = NewObject<UBifrostActionStack>(this);
}

void UBifrostComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void UBifrostComponent::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void UBifrostComponent::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool UBifrostComponent::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool UBifrostComponent::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* UBifrostComponent::GetCurrentAction() const
{
	if (ActionStack)
	{
		ActionStack->GetCurrentAction();
	}
	return nullptr;
}