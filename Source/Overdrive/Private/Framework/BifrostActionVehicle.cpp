// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/BifrostActionVehicle.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"

ABifrostActionVehicle::ABifrostActionVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABifrostActionVehicle::BeginPlay()
{
	Super::BeginPlay();

	ActionStack = NewObject<UBifrostActionStack>(this);
}

void ABifrostActionVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void ABifrostActionVehicle::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void ABifrostActionVehicle::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool ABifrostActionVehicle::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool ABifrostActionVehicle::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* ABifrostActionVehicle::GetCurrentAction() const
{
	if (ActionStack)
	{
		ActionStack->GetCurrentAction();
	}
	return nullptr;
}