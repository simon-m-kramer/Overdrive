// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
#include "Framework/TurboPIDController.h"
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
    // SPEED - PROFILE
    // =========================================================================

    /** Sample interval for the pre-calculated speed profile (cm) */
    UPROPERTY(EditAnywhere, Category = "Speed Profile")
    float SpeedProfileSampleInterval = 100.0f;

    /** Curvature sample range used for speed calculations (cm) */
    UPROPERTY(EditAnywhere, Category = "Speed Profile")
    float SpeedCurvatureSampleRange = 1000.0f;

    /** Safety margin — multiplier on cornering speed (< 1.0 = more cautious; > 1.0 = more aggressive) */
    UPROPERTY(EditAnywhere, Category = "Speed Profile", meta = (ClampMin = "0.5", ClampMax = "1.0"))
    float CorneringSpeedSafetyFactor = 1.0f;

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
    float CoastingThresholdCms = 140.0f;  // ~5 km/h

    /** Throttle applied when coasting within the deadband */
    UPROPERTY(EditAnywhere, Category = "Speed")
    float CoastThrottleInput = 0.15f;

    // =========================================================================
    // FOLLOW DISTANCE
    // =========================================================================

    /** Distance at which we start slowing down for a car ahead (cm) */
    UPROPERTY(EditAnywhere, Category = "Follow Distance")
    float FollowReactionDistance = 3000.0f;

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
    // SPEED AND STEERING CONTROL
    // =========================================================================

    USplineComponent* GetSpline() const;
    float GetLookaheadDistance() const;
    virtual FVector GetTargetPoint();
    float CalculateSteering(const FVector& TargetPoint, float DeltaTime);
    virtual void ApplySpeedControl(float DeltaTime);
    float GetFollowSpeedLimit() const;

    // =========================================================================
    // SPEED PROFILE (pre-calculated)
    // =========================================================================

    void CalculateSpeedProfile();
    float GetTargetSpeedAtDistance(float Distance) const;
    TArray<float> SpeedProfile;  // target speeds in cm/s at each sample point
    bool bSpeedProfileReady = false;

};