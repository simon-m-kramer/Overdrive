// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Yield.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboGameplayTags.h"
#include "Framework/TurboVehicle.h"
#include "Components/TurboVehicleDetectionComponent.h"

UTurboAction_Yield::UTurboAction_Yield()
{
    ActionName = TEXT("Yield");
    ActionTag = TurboGameplayTags::Action_Yield;
    BlocksTags.AddTag(TurboGameplayTags::Action_Yield);
    BlocksTags.AddTag(TurboGameplayTags::Action_Overtake);
    BlocksTags.AddTag(TurboGameplayTags::Action_Evade);

}

bool UTurboAction_Yield::CanActivate(const FTurboDecisionContext& Context) const
{
    return !Context.bLeftClear || !Context.bRightClear;
}

void UTurboAction_Yield::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (bFirstTime)
    {
        TimeInYield = 0.0f;
        TimeSinceClear = 0.0f;
        bYieldComplete = false;

        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(55, 3.0f, FColor::Yellow, TEXT("YIELD STARTED"));
        }
    }
}

void UTurboAction_Yield::Update(float DeltaTime)
{
    TimeInYield += DeltaTime;

    if (IsCarBeside())
    {
        TimeSinceClear = 0.0f;
    }
    else
    {
        TimeSinceClear += DeltaTime;
    }

    Super::Update(DeltaTime);

    if (bDrawDebug)
    {
        GEngine->AddOnScreenDebugMessage(56, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Yield: Time=%.1fs | Clear=%.1fs"),
                TimeInYield, TimeSinceClear));
    }
}

bool UTurboAction_Yield::IsDone()
{
    if (bYieldComplete)
    {
        return true;
    }

    // Timeout
    if (TimeInYield > YieldTimeout)
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(57, 3.0f, FColor::Red, TEXT("YIELD TIMEOUT"));
        }
        bYieldComplete = true;
        return true;
    }

    // Car has been clear for long enough
    if (TimeSinceClear > ClearDuration)
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(57, 3.0f, FColor::Green, TEXT("YIELD COMPLETE - Clear"));
        }
        bYieldComplete = true;
        return true;
    }

    return false;
}

float UTurboAction_Yield::FindTargetSpeedAhead() const
{
    float BaseSpeed = Super::FindTargetSpeedAhead();
    return BaseSpeed - SpeedReductionKmh;
}

bool UTurboAction_Yield::IsCarBeside() const
{
    if (!Vehicle.IsValid())
    {
        return false;
    }

    UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
    if (!Detection)
    {
        return false;
    }

    return Detection->IsCarOnLeft() || Detection->IsCarOnRight();
}

