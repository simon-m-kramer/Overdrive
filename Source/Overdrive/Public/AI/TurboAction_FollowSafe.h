// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_FollowSafe.generated.h"

UCLASS()
class OVERDRIVE_API UTurboAction_FollowSafe : public UTurboAction_FollowPath
{
	GENERATED_BODY()

public:
	UTurboAction_FollowSafe();

	virtual bool IsDone() override;
	virtual bool CanActivate(const FTurboDecisionContext& Context) const override;

	// =========================================================================
	// FOLLOW DISTANCE
	// =========================================================================

	/** Distance at which we start slowing down for a car ahead (cm) */
	UPROPERTY(EditAnywhere, Category = "Follow Distance")
	float FollowReactionDistance = 1500.0f;

	/** Minimum safe following distance (cm) — match their speed at this distance */
	UPROPERTY(EditAnywhere, Category = "Follow Distance")
	float FollowMinDistance = 500.0f;

	/** Below this distance, actively brake harder than the car ahead (cm) */
	UPROPERTY(EditAnywhere, Category = "Follow Distance")
	float FollowEmergencyDistance = 250.0f;

	/** How much slower than the car ahead to go when at min distance (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Follow Distance")
	float FollowSpeedMarginCms = 100.0f;

protected:
	virtual void ApplySpeedControl(float DeltaTime) override;

private:
	float GetFollowSpeedLimit() const;
};
