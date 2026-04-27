// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
#include "Framework/TurboPIDController.h"
#include "Framework/TurboSpeedProfile.h"
#include "TurboAction_FollowPath.generated.h"

class ATurboAIController;
class ATurboVehicle;
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
	// STEERING CONTROL
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Steering")
	FTurboPIDController SteeringPID;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MinLookaheadDistance = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float MaxLookaheadDistance = 2500.0f;

	UPROPERTY(EditAnywhere, Category = "Steering")
	float SteeringLookaheadFactor = 0.7f;  // was LookaheadSpeedFactor

	// =========================================================================
	// SPEED CONTROL
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Speed")
	FTurboPIDController SpeedPID;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float CoastingThresholdCms = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float CoastThrottleInput = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Speed")
	float SpeedLookaheadFactor = 0.7f;

	virtual float GetTargetSpeedAtDistance(float Distance) const;

protected:
	// =========================================================================
	// REFERENCES
	// =========================================================================

	UPROPERTY()
	TWeakObjectPtr<ATurboAIController> AIController;

	UPROPERTY()
	TWeakObjectPtr<ATurboVehicle> Vehicle;

	UPROPERTY()
	TWeakObjectPtr<ATurboRacingSpline> RacingSplineActor;

	UPROPERTY()
	FTurboSpeedProfile SpeedProfile;

	// =========================================================================
	// STEERING AND SPEED
	// =========================================================================

	USplineComponent* GetSpline() const;
	float GetLookaheadDistance() const;
	virtual FVector GetTargetPoint();
	float CalculateSteering(const FVector& TargetPoint, float DeltaTime);
	virtual void ApplySpeedControl(float DeltaTime);



};