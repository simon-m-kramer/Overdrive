// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "AI/TurboActionBase.h"
#include "AI/TurboActionStack.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "AI/TurboAction_FollowPath.h"
#include "Components/SplineComponent.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "AI/TurboAction_GridStart.h"

ATurboAIController::ATurboAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATurboAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    Vehicle = Cast<ATurboVehicle>(InPawn);
    FindRacingSpline();

    InitializeSplineDistance();
    InitializeSpeedProfile();
    InitializeActionStack();
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
// SPEED PROFILE
// =============================================================================

float ATurboAIController::GetTargetSpeedAtDistance(float Distance) const
{
    if (!SpeedProfile.IsReady())
    {
        return 0.0f;
    }
    return SpeedProfile.GetTargetSpeed(Distance);
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
            RacingSpline = SplineActor;
            break;
        }
    }
}

void ATurboAIController::UpdateSplineDistance()
{
    if (!Vehicle || !RacingSpline)
    {
        return;
    }

    USplineComponent* Spline = RacingSpline->GetSplineComponent();
    if (!Spline)
    {
        return;
    }

    FVector VehicleLocation = Vehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

// =============================================================================
// DECISION MAKING
// =============================================================================

void ATurboAIController::UpdateDecisionContext()
{
    if (!Vehicle || !RacingSpline) return;

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

    // Target speed from the active action's speed profile
    DecisionContext.TargetSpeedCms = GetTargetSpeedAtDistance(CurrentSplineDistance);

    // Track analysis - scan ahead for next corner
    DecisionContext.DistanceToNextCorner = 0.0f;

    for (float Look = CornerScanStep; Look < CornerScanMax; Look += CornerScanStep)
    {
        const float Curvature = RacingSpline->GetCurvatureAtDistance(CurrentSplineDistance + Look, CornerScanSampleRange);

        if (Curvature > CornerCurvatureThreshold)
        {
            DecisionContext.DistanceToNextCorner = Look;
            break;
        }
    }

    // Current curvature at vehicle position
    DecisionContext.CurrentCurvature = RacingSpline->GetCurvatureAtDistance(CurrentSplineDistance, CornerScanSampleRange);

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

// =============================================================================
// INITIALIZATION
// =============================================================================

void ATurboAIController::InitializeSpeedProfile()
{
    if (RacingSpline && DrivingProfile)
    {
        SpeedProfile.Calculate(RacingSpline, DrivingProfile);
    }
}

void ATurboAIController::InitializeSplineDistance()
{
    if (!Vehicle || !RacingSpline) return;

    USplineComponent* Spline = RacingSpline->GetSplineComponent();
    if (!Spline) return;

    const FVector VehicleLocation = Vehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

void ATurboAIController::InitializeActionStack()
{
    ActionStack = NewObject<UTurboActionStack>(this);
    if (ActionStack)
    {
        PopulateActionRoster();
        PushDefaultAction();
        PushGridStartAction();
    }
}

void ATurboAIController::PopulateActionRoster()
{
    TArray<UTurboActionBase*> Instances;
    Instances.Reserve(ActionPriorityList.Num());

    for (const TSubclassOf<UTurboActionBase>& ActionClass : ActionPriorityList)
    {
        if (ActionClass)
        {
            Instances.Add(NewObject<UTurboActionBase>(ActionStack, ActionClass.Get()));
        }
    }

    ActionStack->SetActionInstances(Instances);
}

void ATurboAIController::PushDefaultAction()
{
    if (DefaultActionClass)
    {
        UTurboAction_FollowPath* DefaultAction = NewObject<UTurboAction_FollowPath>(ActionStack, DefaultActionClass);
        PushAction(DefaultAction);
    }
}

void ATurboAIController::PushGridStartAction()
{
    UTurboAction_GridStart* GridStart = NewObject<UTurboAction_GridStart>(ActionStack);
    PushAction(GridStart);
}