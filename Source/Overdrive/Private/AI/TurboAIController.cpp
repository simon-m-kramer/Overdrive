// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIController.h"
#include "BifrostActionStack.h"
#include "BifrostAction.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboAction_Overtake.h"
#include "AI/TurboAction_Yield.h"
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
            PreviousSplineDistance = CurrentSplineDistance;
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
    UpdateLapTiming(DeltaTime);
    UpdateDecisionContext();

    // Evaluate and potentially push new actions
    EvaluateActions();

    // Update action stack
    if (ActionStack)
    {
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
            }
        }
        else
        {
            DecisionContext.RelativeSpeedAhead = 0.0f;
        }
    }
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

void ATurboAIController::EvaluateActions()
{
    UBifrostAction* CurrentAction = GetCurrentAction();

    // Don't stack yields
    if (Cast<UTurboAction_Yield>(CurrentAction))
    {
        return;
    }

    if (TryPushYieldAction()) return;

    // Overtake only from FollowPath
    if (Cast<UTurboAction_Overtake>(CurrentAction))
    {
        return;
    }

    if (TryPushOvertakeAction()) return;
}

bool ATurboAIController::TryPushYieldAction()
{
    if (!ControlledVehicle)
    {
        return false;
    }

    // Don't yield if we're the one overtaking (even if overtake is paused in stack)
    const TArray<UBifrostAction*>& Actions = GetActions();
    for (UBifrostAction* Action : Actions)
    {
        if (Cast<UTurboAction_Overtake>(Action))
        {
            return false;
        }
    }

    UTurboVehicleDetectionComponent* Detection = ControlledVehicle->GetDetectionComponent();
    if (!Detection)
    {
        return false;
    }

    if (!Detection->IsCarOnLeft() && !Detection->IsCarOnRight())
    {
        return false;
    }

    UTurboAction_Yield* YieldAction = NewObject<UTurboAction_Yield>(this);
    PushAction(YieldAction);

    if (bShowDecisionContext)
    {
        GEngine->AddOnScreenDebugMessage(41, 2.0f, FColor::Yellow, TEXT("YIELD PUSHED"));
    }

    return true;
}

bool ATurboAIController::TryPushOvertakeAction()
{
    // No car ahead?
    if (!DecisionContext.bCarAhead)
    {
        return false;
    }

    // Too far away?
    if (DecisionContext.DistanceToCarAhead > OvertakeConsiderDistance)
    {
        return false;
    }

    // Not faster than them?
    if (DecisionContext.RelativeSpeedAhead < OvertakeMinSpeedAdvantage)
    {
        return false;
    }

    // Choose side and check if safe
    EOvertakeSide Side = ChooseOvertakeSide();

    // Verify we have clearance
    bool bSideClear = (Side == EOvertakeSide::Left) ? DecisionContext.bLeftClear : DecisionContext.bRightClear;
    if (!bSideClear)
    {
        return false;
    }

    // Check if we'd go off track
    float OvertakeOffset = (Side == EOvertakeSide::Left) ? -OvertakeLateralOffset : OvertakeLateralOffset;
    float ProjectedPosition = DecisionContext.SignedDistanceFromCenter + OvertakeOffset;

    if (FMath::Abs(ProjectedPosition) > DecisionContext.TrackHalfWidth)
    {
        return false;
    }

    // Get target vehicle from detection
    UTurboVehicleDetectionComponent* Detection = ControlledVehicle->GetDetectionComponent();
    if (!Detection)
    {
        return false;
    }

    ATurboVehicle* CarAhead = Detection->GetCarAhead();
    if (!CarAhead)
    {
        return false;
    }

    // All checks passed - create and push overtake action
    UTurboAction_Overtake* OvertakeAction = NewObject<UTurboAction_Overtake>(this);
    OvertakeAction->Initialize(CarAhead, Side);
    OvertakeAction->LateralOffset = OvertakeLateralOffset;
    PushAction(OvertakeAction);

    if (bShowDecisionContext)
    {
        GEngine->AddOnScreenDebugMessage(40, 2.0f, FColor::Green,
            FString::Printf(TEXT("OVERTAKE PUSHED - Side: %s"),
                Side == EOvertakeSide::Left ? TEXT("LEFT") : TEXT("RIGHT")));
    }

    return true;
}

EOvertakeSide ATurboAIController::ChooseOvertakeSide() const
{
    // If in a corner, prefer inside line
    if (!DecisionContext.bOnStraight)
    {
        float TurnSign = RacingSplineActor->GetTurnSign(CurrentSplineDistance, 500.0f);

        // TurnSign > 0 = turning right, inside is left
        // TurnSign < 0 = turning left, inside is right
        if (TurnSign > 0.1f && DecisionContext.bLeftClear)
        {
            return EOvertakeSide::Left;
        }
        else if (TurnSign < -0.1f && DecisionContext.bRightClear)
        {
            return EOvertakeSide::Right;
        }
    }

    // On straight or no clear inside: pick side with more room
    float RoomOnLeft = DecisionContext.TrackHalfWidth + DecisionContext.SignedDistanceFromCenter;
    float RoomOnRight = DecisionContext.TrackHalfWidth - DecisionContext.SignedDistanceFromCenter;

    // Prefer side with more room, but only if clear
    if (RoomOnLeft > RoomOnRight && DecisionContext.bLeftClear)
    {
        return EOvertakeSide::Left;
    }
    else if (DecisionContext.bRightClear)
    {
        return EOvertakeSide::Right;
    }
    else if (DecisionContext.bLeftClear)
    {
        return EOvertakeSide::Left;
    }

    // Default to left if nothing else works
    return EOvertakeSide::Left;
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