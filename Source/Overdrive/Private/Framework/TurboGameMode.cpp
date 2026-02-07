// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboGameMode.h"
#include "Framework/TurboRaceManager.h"

ATurboGameMode::ATurboGameMode()
{
    RaceManager = CreateDefaultSubobject<UTurboRaceManager>(TEXT("RaceManager"));
}