// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
#include "TurboAction_GridStart.generated.h"

class UTurboRaceManager;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboAction_GridStart : public UTurboActionBase
{
	GENERATED_BODY()
	
public:
    UTurboAction_GridStart();

    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

private:
    UPROPERTY()
    TWeakObjectPtr<UTurboRaceManager> RaceManager;
};
