// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Yield.generated.h"

UCLASS()
class OVERDRIVE_API UTurboAction_Yield : public UTurboAction_FollowPath
{
	GENERATED_BODY()

public:
	UTurboAction_Yield();

	virtual void Start(bool bFirstTime) override;
	virtual void Update(float DeltaTime) override;
	virtual bool IsDone() override;
	virtual bool CanActivate(const FTurboDecisionContext& Context) const override;

	// =========================================================================
	// YIELD CONFIGURATION
	// =========================================================================

	/** Lateral offset away from the adjacent car (cm) */
	UPROPERTY(EditAnywhere, Category = "Yield")
	float YieldLateralOffset = 300.0f;

	/** Minimum normalized curvature to trigger yielding (only yield in curves) */
	UPROPERTY(EditAnywhere, Category = "Yield", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinCurvatureToYield = 0.0f;  // 0.001f = roughly a 10m radius corner

protected:
	virtual FVector GetTargetPoint() override;

private:
	/** Which side the adjacent car is on — we move away from this */
	bool bCarOnLeft = false;
	bool bCarOnRight = false;

	bool IsSideClear() const;
};