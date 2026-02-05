// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Sprint.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/TurboVehicleDetectionComponent.h"

UTurboAction_Sprint::UTurboAction_Sprint()
{
    ActionName = TEXT("Sprint");
    ActionTag = TurboGameplayTags::Action_Sprint;
    BlocksTags.AddTag(TurboGameplayTags::Action_Sprint);
}

bool UTurboAction_Sprint::CanActivate(const ATurboAIController* Controller) const
{
    const FTurboDecisionContext& Context = Controller->GetDecisionContext();

    // Need to be on a straight
    if (!Context.bOnStraight)
    {
        return false;
    }

    // Need enough straight road ahead
    if (Context.DistanceToNextCorner < MinStraightDistance)
    {
        return false;
    }

    // Don't sprint if car is close ahead
    if (Context.bCarAhead && Context.DistanceToCarAhead < MinStraightDistance)
    {
        return false;
    }

    return true;
}

void UTurboAction_Sprint::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (bFirstTime)
    {
        TimeInSprint = 0.0f;
        bSprintComplete = false;

        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(58, 3.0f, FColor::Cyan, TEXT("SPRINT STARTED"));
        }
    }
}

void UTurboAction_Sprint::Update(float DeltaTime)
{
    TimeInSprint += DeltaTime;

    Super::Update(DeltaTime);

    if (bDrawDebug)
    {
        GEngine->AddOnScreenDebugMessage(59, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Sprint: Time=%.1fs | Boost=+%.0f km/h"),
                TimeInSprint, SpeedBoostKmh));
    }
}

bool UTurboAction_Sprint::IsDone()
{
    if (bSprintComplete)
    {
        return true;
    }

    if (ShouldExit())
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(59, 3.0f, FColor::Cyan, TEXT("SPRINT COMPLETE"));
        }
        bSprintComplete = true;
        return true;
    }

    return false;
}

float UTurboAction_Sprint::FindTargetSpeedAhead() const
{
    float BaseSpeed = Super::FindTargetSpeedAhead();
    return BaseSpeed + SpeedBoostKmh;
}

bool UTurboAction_Sprint::ShouldExit() const
{
    // Timeout
    if (TimeInSprint > SprintTimeout)
    {
        return true;
    }

    if (!AIController.IsValid())
    {
        return true;
    }

    const FTurboDecisionContext& Context = AIController->GetDecisionContext();

    // Corner approaching
    if (Context.DistanceToNextCorner < MinDistanceToCornerToExit)
    {
        return true;
    }

    // Car ahead too close
    if (Context.bCarAhead && Context.DistanceToCarAhead < MinDistanceToCarAhead)
    {
        return true;
    }

    return false;
}