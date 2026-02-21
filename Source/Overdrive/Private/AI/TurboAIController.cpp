// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "AI/TurboActionBase.h"
#include "AI/TurboActionStack.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "AI/TurboAIVehicle.h"
#include "AI/TurboAction_FollowPath.h"
#include "Components/SplineComponent.h"
#include "AI/TurboVehicleDetectionComponent.h"

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

    // Initialize spline and vehicle
    FindRacingSpline();
    Vehicle = Cast<ATurboAIVehicle>(InPawn);

    // Initialize CurrentSplineDistance and PreviousSplineDistance
    if (Vehicle && RacingSplineActor)
    {
        if (USplineComponent* Spline = RacingSplineActor->GetSplineComponent())
        {
            FVector VehicleLocation = Vehicle->GetActorLocation();
            CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
            PreviousSplineDistance = CurrentSplineDistance;
        }
    }

    // Create ActionStack and ActionPriorityList pass through
    ActionStack = NewObject<UTurboActionStack>(this);
    if (ActionStack)
    {
        ActionStack->ActionPriorityList = ActionPriorityList;
    }

    // Push the default follow path action
    if (DefaultActionClass)
    {
        UTurboAction_FollowPath* DefaultAction = NewObject<UTurboAction_FollowPath>(ActionStack, DefaultActionClass);
        PushAction(DefaultAction);
    }

}

void ATurboAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update shared state
    UpdateSplineDistance();
    UpdateDecisionContext();

    // Update action stack
    if (ActionStack)
    {
        ActionStack->EvaluateActions(DecisionContext);
        ActionStack->UpdateActions(DeltaTime);
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
    if (!Vehicle || !RacingSplineActor)
    {
        return;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return;
    }

    PreviousSplineDistance = CurrentSplineDistance;

    FVector VehicleLocation = Vehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

// =============================================================================
// DECISION MAKING
// =============================================================================

void ATurboAIController::UpdateDecisionContext()
{
    if (!Vehicle || !RacingSplineActor) return;

    UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
    if (!Detection) return;

    // Detection data
    DecisionContext.bVehicleAhead = Detection->IsCarAhead();
    DecisionContext.DistanceToVehicleAhead = Detection->GetDistanceToCarAhead();
    DecisionContext.bVehicleOnLeft = Detection->IsCarOnLeft();
    DecisionContext.bVehicleOnRight = Detection->IsCarOnRight();
    DecisionContext.bVehicleBehind = Detection->IsCarBehind();

    // Speed comparison
    DecisionContext.CurrentSpeedCms = FMath::Abs(Vehicle->GetForwardSpeed());
    DecisionContext.CurrentSplineDistance = CurrentSplineDistance;

    ATurboVehicle* Ahead = Detection->GetCarAhead();
    if (Ahead)
    {
        DecisionContext.SpeedOfVehicleAheadCms = FMath::Abs(Ahead->GetForwardSpeed());
        DecisionContext.SpeedDifferenceCms = DecisionContext.CurrentSpeedCms - DecisionContext.SpeedOfVehicleAheadCms;
    }
    else
    {
        DecisionContext.SpeedOfVehicleAheadCms = 0.0f;
        DecisionContext.SpeedDifferenceCms = 0.0f;
    }

    // Track analysis — scan ahead for next corner
    DecisionContext.DistanceToNextCorner = 0.0f;
    DecisionContext.NextCornerCurvature = 0.0f;

    const float ScanStep = 200.0f;
    const float ScanMax = 10000.0f;

    for (float Look = ScanStep; Look < ScanMax; Look += ScanStep)
    {
        const float Curvature = RacingSplineActor->GetCurvatureAtDistance(
            CurrentSplineDistance + Look, 400.0f);
        const float Normalized = Curvature / FMath::Max(
            RacingSplineActor->GetMaxTrackCurvature(), KINDA_SMALL_NUMBER);

        if (Normalized > RacingSplineActor->GetMinCurvatureThreshold())
        {
            DecisionContext.DistanceToNextCorner = Look;
            DecisionContext.NextCornerCurvature = Normalized;
            break;
        }
    }

    // Target speed from the active action's speed profile
    DecisionContext.TargetSpeedCms = 0.0f;
    if (ActionStack)
    {
        // Find the follow path action (it's always at the bottom of the stack)
        for (UBifrostAction* Action : ActionStack->GetActions())
        {
            UTurboAction_FollowPath* FollowAction = Cast<UTurboAction_FollowPath>(Action);
            if (FollowAction)
            {
                DecisionContext.TargetSpeedCms = FollowAction->GetTargetSpeedAtDistance(CurrentSplineDistance);
                break;
            }
        }
    }

    // CurrentCurvature
    const float RawCurvature = RacingSplineActor->GetCurvatureAtDistance(CurrentSplineDistance, 400.0f);
    DecisionContext.CurrentCurvature = RawCurvature / FMath::Max(RacingSplineActor->GetMaxTrackCurvature(), KINDA_SMALL_NUMBER);

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

const TArray<UBifrostAction*>& ATurboAIController::GetActions() const
{
    static TArray<UBifrostAction*> EmptyArray;
    if (ActionStack)
    {
        return ActionStack->GetActions();
    }
    return EmptyArray;
}

