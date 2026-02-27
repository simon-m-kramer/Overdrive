// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_GridStart.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TurboGameMode.h"
#include "Framework/TurboGameplayTags.h"

UTurboAction_GridStart::UTurboAction_GridStart()
{
    ActionName = TEXT("GridStart");
    ActionTag = TurboGameplayTags::Action_GridStart;

    BlocksTags.AddTag(TurboGameplayTags::Action_GridStart);
    BlocksTags.AddTag(TurboGameplayTags::Action_Overtake);
    BlocksTags.AddTag(TurboGameplayTags::Action_FollowSafe);
}

void UTurboAction_GridStart::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (UWorld* World = GetWorld())
    {
        if (ATurboGameMode* GameMode = Cast<ATurboGameMode>(World->GetAuthGameMode()))
        {
            RaceManager = GameMode->GetRaceManager();
        }
    }
}

void UTurboAction_GridStart::Update(float DeltaTime)
{
    // Hold the car still
    ATurboAIController* AI = GetTypedOuter<ATurboAIController>();
    if (AI)
    {
        ATurboVehicle* Vehicle = AI->GetVehicle();
        if (Vehicle)
        {
            Vehicle->SetThrottleInput(0.0f);
            Vehicle->SetBrakeInput(1.0f);
            Vehicle->SetSteeringInput(0.0f);
            Vehicle->SetHandbrakeInput(true);
        }
    }
}

bool UTurboAction_GridStart::IsDone()
{
    return RaceManager.IsValid() && RaceManager->HasRaceStarted();
}

