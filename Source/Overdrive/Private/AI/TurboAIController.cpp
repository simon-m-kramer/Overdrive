// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "AI/TurboActionBase.h"
#include "AI/TurboActionStack.h"
#include "BifrostActionStack.h"  // old?
#include "BifrostAction.h"  // old?
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboAction_Apex.h"
#include "AI/TurboAction_Overtake.h"
#include "AI/TurboAction_Yield.h"
#include "AI/TurboAction_Sprint.h"
#include "Components/SplineComponent.h"
#include "Components/TurboVehicleDetectionComponent.h"

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
    //UTurboAction_FollowPath* DefaultAction = NewObject<UTurboAction_FollowPath>(ActionStack);
    //PushAction(DefaultAction);

    UTurboAction_Apex* DefaultAction = NewObject<UTurboAction_Apex>(ActionStack);
    PushAction(DefaultAction);

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
        RacingSplineActor->DrawDebugTrackBoundaries(GetWorld());
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

    // Decision context debug
    if (bShowDecisionContext)
    {
        if (ControlledVehicle)
        {
            GEngine->AddOnScreenDebugMessage(34, 0.0f, FColor::White,
                FString::Printf(TEXT("Speed: %.1f km/h"), ControlledVehicle->GetSpeedKmh()));
        }

        GEngine->AddOnScreenDebugMessage(30, 0.0f, FColor::Magenta,
            FString::Printf(TEXT("On Straight: %s | Curvature: %.6f"),
                DecisionContext.bOnStraight ? TEXT("YES") : TEXT("NO"),
                DecisionContext.CurrentCurvature));

        GEngine->AddOnScreenDebugMessage(31, 0.0f, FColor::Magenta,
            FString::Printf(TEXT("Corner in: %.0f cm"), DecisionContext.DistanceToNextCorner));

        if (DecisionContext.bCarAhead)
        {
            GEngine->AddOnScreenDebugMessage(32, 0.0f, FColor::Orange,
                FString::Printf(TEXT("Car Ahead: %.0f cm | Rel Speed: %.1f km/h"),
                    DecisionContext.DistanceToCarAhead,
                    DecisionContext.RelativeSpeedAhead));
        }

        GEngine->AddOnScreenDebugMessage(33, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Clear L: %s | R: %s | Pos: %.0f cm"),
                DecisionContext.bLeftClear ? TEXT("YES") : TEXT("NO"),
                DecisionContext.bRightClear ? TEXT("YES") : TEXT("NO"),
                DecisionContext.SignedDistanceFromCenter));

        // Action stack
        UBifrostAction* Active = GetCurrentAction();
        if (Active)
        {
            GEngine->AddOnScreenDebugMessage(60, 0.0f, FColor::White,
                FString::Printf(TEXT("* %s"), *Active->ActionName));
        }

        const TArray<UBifrostAction*>& Actions = GetActions();
        for (int32 i = 0; i < Actions.Num(); i++)
        {
            if (Actions[i])
            {
                GEngine->AddOnScreenDebugMessage(61 + i, 0.0f, FColor::Silver,
                    FString::Printf(TEXT("  [%d] %s"), i, *Actions[i]->ActionName));
            }
        }


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
// DECISION MAKING
// =============================================================================

void ATurboAIController::UpdateDecisionContext()
{
    if (!ControlledVehicle || !RacingSplineActor)
    {
        return;
    }

    // Track geometry
    DecisionContext.CurrentCurvature = RacingSplineActor->GetCurvatureAtDistance(CurrentSplineDistance, 300.0f);
    DecisionContext.bOnStraight = DecisionContext.CurrentCurvature < StraightCurvatureThreshold;
    DecisionContext.DistanceToNextCorner = FindDistanceToNextCorner();

    // Track position
    FVector VehicleLocation = ControlledVehicle->GetActorLocation();
    DecisionContext.SignedDistanceFromCenter = RacingSplineActor->GetSignedDistanceFromCenter(VehicleLocation);
    DecisionContext.TrackHalfWidth = RacingSplineActor->TrackWidth * 0.5f;

    // Detection component data
    UTurboVehicleDetectionComponent* Detection = ControlledVehicle->GetDetectionComponent();
    if (Detection)
    {
        DecisionContext.bCarAhead = Detection->IsCarAhead();
        DecisionContext.DistanceToCarAhead = Detection->GetDistanceToCarAhead();
        DecisionContext.bLeftClear = !Detection->IsCarOnLeft();
        DecisionContext.bRightClear = !Detection->IsCarOnRight();

        // Calculate relative speed
        if (DecisionContext.bCarAhead)
        {
            ATurboVehicle* CarAhead = Detection->GetCarAhead();
            if (CarAhead)
            {
                float OurSpeed = ControlledVehicle->GetSpeedKmh();
                float TheirSpeed = CarAhead->GetSpeedKmh();
                DecisionContext.RelativeSpeedAhead = OurSpeed - TheirSpeed;
                DecisionContext.CarAhead = Detection->GetCarAhead();
            }
        }
        else
        {
            DecisionContext.RelativeSpeedAhead = 0.0f;
            DecisionContext.CarAhead = nullptr;
        }
    }

    DecisionContext.CurrentTurnSign = RacingSplineActor->GetTurnSign(CurrentSplineDistance, 500.0f);


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

