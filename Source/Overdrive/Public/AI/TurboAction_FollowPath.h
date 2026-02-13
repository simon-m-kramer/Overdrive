// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
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

    /** Safety margin — multiplier on cornering speed (< 1.0 = more cautious) */
    UPROPERTY(EditAnywhere, Category = "Speed Profile", meta = (ClampMin = "0.5", ClampMax = "1.0"))
    float CorneringSpeedSafetyFactor = 0.9f;

    // =========================================================================
    // STEERING CONTROL
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Lookahead")
    float MinLookaheadDistance = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Lookahead")
    float MaxLookaheadDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Lookahead")
    float LookaheadSpeedFactor = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Turbo AI|Steering")
    float SteeringGain = 2.0f;

    // =========================================================================
    // SPEED CONTROL
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float CoastingThresholdKmh = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float BrakeProportionalGain = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float ThrottleProportionalGain = 0.03f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float MinThrottleInput = 0.2f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float MinBrakeInput = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float MaxThrottleInput = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float MaxBrakeInput = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control|Tuning")
    float CoastThrottleInput = 0.15f;

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
    float CalculateSteering(const FVector& TargetPoint);
    virtual void ApplySpeedControl();

    // =========================================================================
    // SPEED PROFILE (pre-calculated)
    // =========================================================================

    void CalculateSpeedProfile();
    float GetTargetSpeedAtDistance(float Distance) const;
    TArray<float> SpeedProfile;  // target speeds in cm/s at each sample point
    bool bSpeedProfileReady = false;

    /*
    float FindTargetSpeedAhead();  // Alternative to calculate speed profile
    */

};