// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Overtake.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Components/SplineComponent.h"

void UTurboAction_Overtake::Initialize(ATurboVehicle* InTargetVehicle, EOvertakeSide InSide)
{
    TargetVehicle = InTargetVehicle;
    Side = InSide;
}

void UTurboAction_Overtake::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    ActionName = TEXT("Overtake");

    if (bFirstTime)
    {
        CurrentLateralOffset = 0.0f;
        TimeInOvertake = 0.0f;
        bOvertakeComplete = false;
        bHasPassedTarget = false;
        TimeSincePassed = 0.0f;

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
    if (TimeInOvertake > AbortTimeout)
    {
        return true;
    }

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

    // Reduce boost during hold phase
    if (bHasPassedTarget)
    {
        float HoldAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
        return BaseSpeed + SpeedBoostKmh * (1.0f - HoldAlpha);
    }

    return BaseSpeed + SpeedBoostKmh;
}