// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "AI/TurboAction_FollowPath.h"
#include "Components/SplineComponent.h"

ATurboAIController::ATurboAIController()
{
    PrimaryActorTick.bCanEverTick = true;
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

    // Initialize spline distance
    if (ControlledVehicle && RacingSplineActor)
    {
        USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
        if (Spline)
        {
            FVector VehicleLocation = ControlledVehicle->GetActorLocation();
            CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
        }
    }

    // Push the default follow path action
    UTurboAction_FollowPath* DefaultAction = NewObject<UTurboAction_FollowPath>(this);
    PushAction(DefaultAction);
}

void ATurboAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update shared state
    UpdateSplineDistance();

    // Update action stack
    if (ActionStack)
    {
        ActionStack->UpdateActions(DeltaTime);
    }

    // Debug visualization
    if (bDrawDebug && RacingSplineActor)
    {
        RacingSplineActor->DrawDebugRacingLine(GetWorld());
    }
}

// =============================================================================
// SPLINE & POSITION
// =============================================================================

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

void ATurboAIController::UpdateSplineDistance()
{
    if (!ControlledVehicle || !RacingSplineActor)
    {
        return;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return;
    }

    FVector VehicleLocation = ControlledVehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

// =============================================================================
// ACTION STACK
// =============================================================================

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
