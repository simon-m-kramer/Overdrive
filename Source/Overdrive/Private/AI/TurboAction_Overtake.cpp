// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Overtake.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Components/TurboVehicleDetectionComponent.h"

UTurboAction_Overtake::UTurboAction_Overtake()
{
    ActionName = TEXT("Overtake");
    ActionTag = TurboGameplayTags::Action_Overtake;
    BlocksTags.AddTag(TurboGameplayTags::Action_Overtake);
    BlocksTags.AddTag(TurboGameplayTags::Action_Yield);
}

bool UTurboAction_Overtake::CanActivate(const ATurboAIController* Controller) const
{
    const FTurboDecisionContext& Context = Controller->GetDecisionContext();

    // No car ahead?
    if (!Context.bCarAhead)
    {
        return false;
    }

    // Too far away?
    if (Context.DistanceToCarAhead > ConsiderDistance)
    {
        return false;
    }

    // Not faster than them?
    if (Context.RelativeSpeedAhead < MinSpeedAdvantage)
    {
        return false;
    }

    // Choose side and check if safe
    EOvertakeSide ChosenSide = ChooseOvertakeSide(Controller);

    // Verify we have clearance
    bool bSideClear = (ChosenSide == EOvertakeSide::Left) ? Context.bLeftClear : Context.bRightClear;
    if (!bSideClear)
    {
        return false;
    }

    // Check if we'd go off track
    float OvertakeOffset = (ChosenSide == EOvertakeSide::Left) ? -LateralOffset : LateralOffset;
    float ProjectedPosition = Context.SignedDistanceFromCenter + OvertakeOffset;
    if (FMath::Abs(ProjectedPosition) > Context.TrackHalfWidth)
    {
        return false;
    }

    // Verify target vehicle exists
    ATurboVehicle* ControlledVehicle = Controller->GetControlledVehicle();
    if (!ControlledVehicle)
    {
        return false;
    }

    UTurboVehicleDetectionComponent* Detection = ControlledVehicle->GetDetectionComponent();
    if (!Detection || !Detection->GetCarAhead())
    {
        return false;
    }

    return true;
}

void UTurboAction_Overtake::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (bFirstTime)
    {
        CurrentLateralOffset = 0.0f;
        TimeInOvertake = 0.0f;
        bOvertakeComplete = false;
        bHasPassedTarget = false;
        TimeSincePassed = 0.0f;

        // Set up target and side from current context
        if (AIController.IsValid())
        {
            Side = ChooseOvertakeSide(AIController.Get());

            ATurboVehicle* ControlledVehicle = AIController->GetControlledVehicle();
            if (ControlledVehicle)
            {
                UTurboVehicleDetectionComponent* Detection = ControlledVehicle->GetDetectionComponent();
                if (Detection)
                {
                    TargetVehicle = Detection->GetCarAhead();
                }
            }
        }

        // Store target's starting position for comparison
        if (TargetVehicle.IsValid() && RacingSplineActor.IsValid())
        {
            USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
            if (Spline)
            {
                FVector TargetLocation = TargetVehicle->GetActorLocation();
                TargetSplineDistanceAtStart = Spline->GetDistanceAlongSplineAtLocation(TargetLocation, ESplineCoordinateSpace::World);
            }
        }

        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(50, 3.0f, FColor::Green,
                FString::Printf(TEXT("OVERTAKE STARTED - Side: %s"),
                    Side == EOvertakeSide::Left ? TEXT("LEFT") : TEXT("RIGHT")));
        }
    }
}

void UTurboAction_Overtake::Update(float DeltaTime)
{
    TimeInOvertake += DeltaTime;

    if (bHasPassedTarget)
    {
        // Hold phase: blend offset back toward zero
        TimeSincePassed += DeltaTime;

        float BlendOutAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
        float FullOffset = (Side == EOvertakeSide::Left) ? -LateralOffset : LateralOffset;
        CurrentLateralOffset = FMath::Lerp(FullOffset, 0.0f, BlendOutAlpha);
    }
    else
    {
        // Passing phase: blend offset toward target
        float TargetOffset = (Side == EOvertakeSide::Left) ? -LateralOffset : LateralOffset;
        CurrentLateralOffset = FMath::FInterpTo(CurrentLateralOffset, TargetOffset, DeltaTime, OffsetBlendSpeed);

        if (HasPassedTargetVehicle())
        {
            bHasPassedTarget = true;
            TimeSincePassed = 0.0f;
        }
    }

    // Call parent update for steering and speed control
    Super::Update(DeltaTime);

    if (bDrawDebug)
    {
        FString Phase = bHasPassedTarget ? TEXT("HOLD") : TEXT("PASSING");
        GEngine->AddOnScreenDebugMessage(51, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Overtake [%s]: Offset=%.0f | Time=%.1fs"),
                *Phase, CurrentLateralOffset, TimeInOvertake));
    }
}

bool UTurboAction_Overtake::IsDone()
{
    if (bOvertakeComplete)
    {
        return true;
    }

    if (ShouldAbort())
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(52, 3.0f, FColor::Red, TEXT("OVERTAKE ABORTED"));
        }
        bOvertakeComplete = true;
        return true;
    }

    // Complete after hold time expires
    if (bHasPassedTarget && TimeSincePassed >= CompletionHoldTime)
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(52, 3.0f, FColor::Green, TEXT("OVERTAKE COMPLETE"));
        }
        bOvertakeComplete = true;
        return true;
    }

    return false;
}

FVector UTurboAction_Overtake::GetTargetPoint()
{
    // Get the base racing line point from parent
    FVector BasePoint = Super::GetTargetPoint();

    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return BasePoint;
    }

    // Apply lateral offset
    if (FMath::Abs(CurrentLateralOffset) < 1.0f)
    {
        return BasePoint;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return BasePoint;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float LookaheadDist = GetLookaheadDistance();
    float TargetDistance = CurrentDistance + LookaheadDist;

    // Wrap for closed loop
    float SplineLength = Spline->GetSplineLength();
    if (Spline->IsClosedLoop() && TargetDistance >= SplineLength)
    {
        TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
    }

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    // Add offset to base point (positive offset = right, negative = left)
    return BasePoint + (Right * CurrentLateralOffset);
}

EOvertakeSide UTurboAction_Overtake::ChooseOvertakeSide(const ATurboAIController* Controller) const
{
    const FTurboDecisionContext& Context = Controller->GetDecisionContext();
    ATurboRacingSpline* Spline = Controller->GetRacingSplineActor();

    // If in a corner, prefer inside line
    if (!Context.bOnStraight && Spline)
    {
        float TurnSign = Spline->GetTurnSign(Controller->GetCurrentSplineDistance(), 500.0f);

        // TurnSign > 0 = turning right, inside is left
        // TurnSign < 0 = turning left, inside is right
        if (TurnSign > 0.1f && Context.bLeftClear)
        {
            return EOvertakeSide::Left;
        }
        else if (TurnSign < -0.1f && Context.bRightClear)
        {
            return EOvertakeSide::Right;
        }
    }

    // On straight or no clear inside: pick side with more room
    float RoomOnLeft = Context.TrackHalfWidth + Context.SignedDistanceFromCenter;
    float RoomOnRight = Context.TrackHalfWidth - Context.SignedDistanceFromCenter;

    // Prefer side with more room, but only if clear
    if (RoomOnLeft > RoomOnRight && Context.bLeftClear)
    {
        return EOvertakeSide::Left;
    }
    else if (Context.bRightClear)
    {
        return EOvertakeSide::Right;
    }
    else if (Context.bLeftClear)
    {
        return EOvertakeSide::Left;
    }

    // Default to left if nothing else works
    return EOvertakeSide::Left;
}

bool UTurboAction_Overtake::HasPassedTargetVehicle() const
{
    if (!TargetVehicle.IsValid() || !AIController.IsValid() || !RacingSplineActor.IsValid())
    {
        return true;  // Target lost, consider done
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return true;
    }

    // Get current positions on spline
    float OurDistance = AIController->GetCurrentSplineDistance();

    FVector TargetLocation = TargetVehicle->GetActorLocation();
    float TargetDistance = Spline->GetDistanceAlongSplineAtLocation(TargetLocation, ESplineCoordinateSpace::World);

    // Handle lap wrap-around
    float SplineLength = Spline->GetSplineLength();
    float DistanceAhead = OurDistance - TargetDistance;

    if (Spline->IsClosedLoop())
    {
        if (DistanceAhead < -SplineLength * 0.5f)
        {
            DistanceAhead += SplineLength;
        }
        else if (DistanceAhead > SplineLength * 0.5f)
        {
            DistanceAhead -= SplineLength;
        }
    }

    return DistanceAhead > MinDistanceAheadToComplete;
}

bool UTurboAction_Overtake::ShouldAbort() const
{
    // Timeout
    if (TimeInOvertake > AbortTimeout)
    {
        return true;
    }

    // Target vehicle lost
    if (!TargetVehicle.IsValid())
    {
        return true;
    }

    // Abort if entering a tight corner mid-overtake (not during hold)
    if (!bHasPassedTarget && AIController.IsValid() && RacingSplineActor.IsValid())
    {
        float CurrentCurvature = RacingSplineActor->GetCurvatureAtDistance(
            AIController->GetCurrentSplineDistance(), 500.0f);

        if (CurrentCurvature > CornerAbortCurvature)
        {
            return true;
        }
    }

    return false;
}

float UTurboAction_Overtake::FindTargetSpeedAhead() const
{
    float BaseSpeed = Super::FindTargetSpeedAhead();

    // No boost in corners
    if (AIController.IsValid() && RacingSplineActor.IsValid())
    {
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(
            AIController->GetCurrentSplineDistance(), 500.0f);

        if (Curvature > 0.0003f)
        {
            // In a corner — use base speed, no boost
            if (bHasPassedTarget)
            {
                float HoldAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
                return BaseSpeed * (1.0f - HoldAlpha * 0.05f);
            }
            return BaseSpeed;
        }
    }

    // On straight — full boost
    if (bHasPassedTarget)
    {
        float HoldAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
        return BaseSpeed + SpeedBoostKmh * (1.0f - HoldAlpha);
    }

    return BaseSpeed + SpeedBoostKmh;
}