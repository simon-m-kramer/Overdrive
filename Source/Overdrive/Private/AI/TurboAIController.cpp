// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"


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
		// Check if this specific actor has the MainSpline tag
		if (SplineActor && SplineActor->GetGameplayTags().HasTag(TurboGameplayTags::Track_MainSpline))
		{
			RacingSpline = SplineActor->GetSplineComponent();
			break;
		}
	}
}

void ATurboAIController::BeginPlay()
{
	Super::BeginPlay();

	ActionStack = NewObject<UBifrostActionStack>(this);
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
		ActionStack->GetCurrentAction();
	}
	return nullptr;
}
