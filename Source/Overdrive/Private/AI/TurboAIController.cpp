// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "AI/TurboAction_FollowPath.h"


ATurboAIController::ATurboAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATurboAIController::FindRacingSpline()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurboRacingSpline::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ATurboRacingSpline* SplineActor = Cast<ATurboRacingSpline>(Actor);
		if (SplineActor && SplineActor->GetGameplayTags().HasTag(TurboGameplayTags::Track_MainSpline))
		{
			RacingSplineActor = SplineActor;
			break;
		}
	}
}

void ATurboAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ATurboAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ActionStack = NewObject<UBifrostActionStack>(this);
	FindRacingSpline();
	ControlledVehicle = Cast<ATurboVehicle>(InPawn);

	// Push the default follow path action
	UTurboAction_FollowPath* DefaultAction = NewObject<UTurboAction_FollowPath>(this);
	PushAction(DefaultAction);
}

void ATurboAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActionStack)
	{
		ActionStack->UpdateActions(DeltaTime);
	}
}

void ATurboAIController::PushAction(UBifrostAction* NewAction)
{
	if (ActionStack)
	{
		ActionStack->PushAction(NewAction);
	}
}

void ATurboAIController::RemoveAction(UBifrostAction* InAction)
{
	if (ActionStack)
	{
		ActionStack->RemoveAction(InAction);
	}
}

bool ATurboAIController::Contains(UBifrostAction* InAction)
{
	return ActionStack && ActionStack->Contains(InAction);
}

bool ATurboAIController::IsEmpty() const
{
	return ActionStack && ActionStack->IsEmpty();
}

UBifrostAction* ATurboAIController::GetCurrentAction() const
{
	if (ActionStack)
	{
		return ActionStack->GetCurrentAction();
	}
	return nullptr;
}
