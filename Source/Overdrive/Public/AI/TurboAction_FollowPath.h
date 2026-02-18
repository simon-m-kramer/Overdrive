// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
#include "Framework/TurboPIDController.h"
#include "Framework/TurboSpeedProfile.h"
#include "TurboAction_FollowPath.generated.h"

class ATurboAIController;
class ATurboAIVehicle;
class ATurboRacingSpline;
class USplineComponent;

UCLASS()
class OVERDRIVE_API UTurboAction_FollowPath : public UTurboActionBase
{
	GENERATED_BODY()

public:
	UTurboAction_FollowPath();

	virtual void Start(bool bFirstTime) override;
	virtual void Update(float DeltaTime) override;
	virtual bool IsDone() override { return false; }

	// =========================================================================
	// SPEED PROFILE
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Speed Profile")
	FTurboSpeedProfile SpeedProfile;

	// =========================================================================
	// STEERING CONTROL
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Steering")
	FTurboPIDController SteeringPID;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MinLookaheadDistance = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MaxLookaheadDistance = 2500.0f;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float LookaheadSpeedFactor = 0.5f;

	// =========================================================================
	// SPEED CONTROL
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Speed")
	FTurboPIDController SpeedPID;

	/** Deadband in cm/s — within this range, coast instead of correcting */
	UPROPERTY(EditAnywhere, Category = "Speed")
	float CoastingThresholdCms = 50.0f;

	/** Throttle applied when coasting within the deadband */
	UPROPERTY(EditAnywhere, Category = "Speed")
	float CoastThrottleInput = 0.25f;

	float GetTargetSpeedAtDistance(float Distance) const;

protected:
	// =========================================================================
	// REFERENCES
	// =========================================================================

	UPROPERTY()
	TWeakObjectPtr<ATurboAIController> AIController;

	UPROPERTY()
	TWeakObjectPtr<ATurboAIVehicle> Vehicle;

	UPROPERTY()
	TWeakObjectPtr<ATurboRacingSpline> RacingSplineActor;

	// =========================================================================
	// STEERING AND SPEED
	// =========================================================================

	USplineComponent* GetSpline() const;
	float GetLookaheadDistance() const;
	virtual FVector GetTargetPoint();
	float CalculateSteering(const FVector& TargetPoint, float DeltaTime);
	virtual void ApplySpeedControl(float DeltaTime);

};