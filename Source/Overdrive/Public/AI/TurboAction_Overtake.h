// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "TurboAction_Overtake.generated.h"

UCLASS()
class OVERDRIVE_API UTurboAction_Overtake : public UTurboAction_FollowPath
{
	GENERATED_BODY()

public:
	UTurboAction_Overtake();

	virtual void Start(bool bFirstTime) override;
	virtual void Update(float DeltaTime) override;
	virtual bool IsDone() override;
	virtual bool CanActivate(const FTurboDecisionContext& Context) const override;

	// =========================================================================
	// OVERTAKE TRIGGER
	// =========================================================================

	/** How close to the car ahead before considering an overtake (cm) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Trigger")
	float OvertakeConsiderationDistance = 2500.0f;

	/** Minimum speed advantage from profile vs car ahead to trigger (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Trigger")
	float MinSpeedAdvantage = 200.0f;  // 300.0 = ~11 km/h faster

	/** Minimum straight distance ahead needed to attempt a pass (cm) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Trigger")
	float MinStraightNeeded = 1500.0f;

	// =========================================================================
	// OVERTAKE BEHAVIOR
	// =========================================================================

	/** Lateral offset from racing line when passing (cm) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Behavior")
	float OvertakeLateralOffset = 400.0f;

	/** Extra speed boost as fraction of max speed during overtake */
	UPROPERTY(EditAnywhere, Category = "Overtake|Behavior")
	float OvertakeSpeedBoost = 1.05f;

	/** How long the overtake can last before aborting (seconds) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Behavior")
	float OvertakeTimeout = 8.0f;

	// =========================================================================
	// COMPLETION
	// =========================================================================

	/** How far ahead of the overtaken car we need to be to consider the pass complete (cm) */
	UPROPERTY(EditAnywhere, Category = "Overtake|Completion")
	float ClearanceDistanceAhead = 800.0f;

protected:
	virtual FVector GetTargetPoint() override;
	virtual void ApplySpeedControl(float DeltaTime) override;

private:
	EOvertakeSide ChosenSide = EOvertakeSide::Left;
	bool bOvertakeComplete = false;
	bool bOvertakeAborted = false;
	float OvertakeTimer = 0.0f;

	/** The vehicle we're trying to pass — stored at Start so we can track it */
	UPROPERTY()
	TWeakObjectPtr<ATurboVehicle> OvertakeTarget;

	EOvertakeSide DetermineBestSide() const;
	bool IsPassComplete() const;
	bool ShouldAbort() const;
};
