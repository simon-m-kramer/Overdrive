// Copyright Simon Kramer. All Rights Reserved.

/*
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
    BlocksTags.AddTag(TurboGameplayTags::Action_Evade);
}

bool UTurboAction_Overtake::CanActivate(const FTurboDecisionContext& Context) const
{
    if (!Context.bCarAhead) return false;
    if (Context.DistanceToCarAhead > ConsiderDistance) return false;
    if (Context.RelativeSpeedAhead < MinSpeedAdvantage) return false;
    if (Context.CurrentCurvature > OvertakeMaxCurvature) return false;
    if (!Context.CarAhead.IsValid()) return false;

    return true;
}

void UTurboAction_Overtake::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (bFirstTime)
    {
        LaneBlendAlpha = 0.0f;
        TimeInOvertake = 0.0f;
        bOvertakeComplete = false;
        bHasPassedTarget = false;
        TimeSincePassed = 0.0f;

        if (AIController.IsValid())
        {
            const FTurboDecisionContext& Context = AIController->GetDecisionContext();
            TargetVehicle = Context.CarAhead;
        }

        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(50, 3.0f, FColor::Green, TEXT("OVERTAKE STARTED — switching to secondary lane"));
        }
    }
}

void UTurboAction_Overtake::Update(float DeltaTime)
{
    TimeInOvertake += DeltaTime;

    if (bHasPassedTarget)
    {
        TimeSincePassed += DeltaTime;

        // Blend back to primary line
        LaneBlendAlpha = FMath::FInterpConstantTo(LaneBlendAlpha, 0.0f, DeltaTime, LaneBlendSpeed);
    }
    else
    {
        // Blend toward secondary line
        LaneBlendAlpha = FMath::FInterpConstantTo(LaneBlendAlpha, 1.0f, DeltaTime, LaneBlendSpeed);

        if (HasPassedTargetVehicle())
        {
            bHasPassedTarget = true;
            TimeSincePassed = 0.0f;
        }
    }

    Super::Update(DeltaTime);

    if (bDrawDebug)
    {
        FString Phase = bHasPassedTarget ? TEXT("HOLD") : TEXT("PASSING");
        GEngine->AddOnScreenDebugMessage(51, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Overtake [%s]: Blend=%.2f | Time=%.1fs"),
                *Phase, LaneBlendAlpha, TimeInOvertake));
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

    // Complete after hold time and fully blended back to primary
    if (bHasPassedTarget && TimeSincePassed >= CompletionHoldTime && LaneBlendAlpha < 0.01f)
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
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return Super::GetTargetPoint();
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float LookaheadDist = GetLookaheadDistance();
    float TargetDistance = CurrentDistance + LookaheadDist;

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (Spline && Spline->IsClosedLoop())
    {
        float SplineLength = Spline->GetSplineLength();
        if (TargetDistance >= SplineLength)
        {
            TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
        }
    }

    FVector PrimaryPoint = RacingSplineActor->GetPointOnRacingLine(TargetDistance);
    FVector SecondaryPoint = RacingSplineActor->GetPointOnSecondaryLine(TargetDistance);

    return FMath::Lerp(PrimaryPoint, SecondaryPoint, LaneBlendAlpha);
}

bool UTurboAction_Overtake::HasPassedTargetVehicle() const
{
    if (!TargetVehicle.IsValid() || !AIController.IsValid() || !RacingSplineActor.IsValid())
    {
        return true;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return true;
    }

    float OurDistance = AIController->GetCurrentSplineDistance();

    FVector TargetLocation = TargetVehicle->GetActorLocation();
    float TargetDistance = Spline->GetDistanceAlongSplineAtLocation(TargetLocation, ESplineCoordinateSpace::World);

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

    return false;
}

float UTurboAction_Overtake::FindTargetSpeedAhead() const
{
    float BaseSpeed = Super::FindTargetSpeedAhead();

    if (!AIController.IsValid() || !RacingSplineActor.IsValid())
    {
        return BaseSpeed;
    }

    float Curvature = RacingSplineActor->GetCurvatureAtDistance(
        AIController->GetCurrentSplineDistance(), 500.0f);

    // No boost in corners
    if (Curvature > 0.0003f)
    {
        if (bHasPassedTarget)
        {
            float HoldAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
            return BaseSpeed * (1.0f - HoldAlpha * 0.05f);
        }
        return BaseSpeed;
    }

    // On straights — boost
    if (bHasPassedTarget)
    {
        float HoldAlpha = FMath::Clamp(TimeSincePassed / CompletionHoldTime, 0.0f, 1.0f);
        return BaseSpeed + SpeedBoostKmh * (1.0f - HoldAlpha);
    }

    return BaseSpeed + SpeedBoostKmh;
}
*/