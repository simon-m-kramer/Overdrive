// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
#include "TurboAction_ProfileSwap.generated.h"

class UTurboDrivingProfile;
/**
 * 
 */
UCLASS()
class OVERDRIVE_API UTurboAction_ProfileSwap : public UTurboActionBase
{
	GENERATED_BODY()
	
public:
	virtual void Start(bool bFirstTime) override;
	virtual bool IsDone() override;
	virtual bool CanActivate(const FTurboDecisionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Profile")
	TObjectPtr<UTurboDrivingProfile> TargetProfile;

};
