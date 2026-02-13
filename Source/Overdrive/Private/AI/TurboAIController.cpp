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

    // ActionPriorityList and DisabledActions pass through
    ActionStack = NewObject<UTurboActionStack>(this);
    if (ActionStack)
    {
        ActionStack->ActionPriorityList = ActionPriorityList;
        ActionStack->DisabledActions = DisabledActions;
    }

    // Initialize spline and vehicle
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
            PreviousSplineDistance = CurrentSplineDistance;
        }
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
    UpdateLapTiming(DeltaTime);
    UpdateDecisionContext();

    // Update action stack
    if (ActionStack)
    {
        ActionStack->EvaluateActions(DecisionContext);
        ActionStack->UpdateActions(DeltaTime);
    }

    // Debug visualization
    if (bDrawDebug && RacingSplineActor)
    {
        RacingSplineActor->DrawDebugRacingLine(GetWorld());
    }

    // Lap timing display
    if (bShowLapTiming)
    {
        GEngine->AddOnScreenDebugMessage(10, 0.0f, FColor::White,
            FString::Printf(TEXT("Current Lap: %s"), *FormatLapTime(CurrentLapTime)));

        if (LastLapTime > 0.0f)
        {
            GEngine->AddOnScreenDebugMessage(11, 0.0f, FColor::Yellow,
                FString::Printf(TEXT("Last Lap: %s"), *FormatLapTime(LastLapTime)));
        }

        if (BestLapTime > 0.0f)
        {
            GEngine->AddOnScreenDebugMessage(12, 0.0f, FColor::Green,
                FString::Printf(TEXT("Best Lap: %s"), *FormatLapTime(BestLapTime)));
        }

        GEngine->AddOnScreenDebugMessage(13, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Lap: %d"), LapCount));
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

    PreviousSplineDistance = CurrentSplineDistance;

    FVector VehicleLocation = ControlledVehicle->GetActorLocation();
    CurrentSplineDistance = Spline->GetDistanceAlongSplineAtLocation(VehicleLocation, ESplineCoordinateSpace::World);
}

// =============================================================================
// DECISION MAKING
// =============================================================================

void ATurboAIController::UpdateDecisionContext()
{

}

float ATurboAIController::FindDistanceToNextCorner() const
{
    if (!RacingSplineActor)
    {
        return CornerScanDistance;
    }

    for (float Ahead = 0.0f; Ahead < CornerScanDistance; Ahead += 200.0f)
    {
        float ScanDist = CurrentSplineDistance + Ahead;
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(ScanDist, 300.0f);

        if (Curvature > StraightCurvatureThreshold)
        {
            return Ahead;
        }
    }

    return CornerScanDistance;
}

// =============================================================================
// LAP TIMING
// =============================================================================

void ATurboAIController::UpdateLapTiming(float DeltaTime)
{
    if (!RacingSplineActor)
    {
        return;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline || !Spline->IsClosedLoop())
    {
        return;
    }

    float SplineLength = Spline->GetSplineLength();

    // Detect lap completion: distance wrapped from high to low
    bool bCrossedFinishLine = (PreviousSplineDistance > SplineLength * 0.9f) &&
        (CurrentSplineDistance < SplineLength * 0.1f);

    if (bCrossedFinishLine)
    {
        if (bLapTimingStarted)
        {
            // Completed a lap
            LastLapTime = CurrentLapTime;

            if (BestLapTime <= 0.0f || CurrentLapTime < BestLapTime)
            {
                BestLapTime = CurrentLapTime;
            }

            LapCount++;
        }

        // Start/restart lap timer
        CurrentLapTime = 0.0f;
        bLapTimingStarted = true;
    }
    else if (bLapTimingStarted)
    {
        CurrentLapTime += DeltaTime;
    }
}

void ATurboAIController::ResetLapTiming()
{
    CurrentLapTime = 0.0f;
    LastLapTime = 0.0f;
    BestLapTime = 0.0f;
    LapCount = 0;
    bLapTimingStarted = false;
}

FString ATurboAIController::FormatLapTime(float TimeSeconds) const
{
    int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
    float Seconds = FMath::Fmod(TimeSeconds, 60.0f);

    return FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
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

