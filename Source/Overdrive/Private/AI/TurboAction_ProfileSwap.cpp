// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_ProfileSwap.h"
#include "Framework/TurboDrivingProfile.h"
#include "AI/TurboAIController.h"

void UTurboAction_ProfileSwap::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);
}

bool UTurboAction_ProfileSwap::IsDone()
{
	return true;
}

bool UTurboAction_ProfileSwap::CanActivate(const FTurboDecisionContext& Context) const
{
	return false;
}
