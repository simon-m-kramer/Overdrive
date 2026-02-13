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
    // STEERING CONFIGURATION
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
    // SPEED - LIMITS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float MaxSpeedKmh = 240.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float MinCornerSpeedKmh = 90.0f;

    // =========================================================================
    // SPEED - PHYSICS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float GripFactor = 2800.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float SpeedCurvatureSampleRange = 1000.0f;

    // =========================================================================
    // SPEED - CORNER SCANNING
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float CornerScanDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float CornerScanInterval = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float DistanceSpeedBuffer = 50.0f;

    // =========================================================================
    // SPEED - INPUT TUNING
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
    // STEERING
    // =========================================================================

    USplineComponent* GetSpline() const;
    float GetLookaheadDistance() const;
    virtual FVector GetTargetPoint();
    float CalculateSteering(const FVector& TargetPoint);

    // =========================================================================
    // SPEED
    // =========================================================================

    virtual float FindTargetSpeedAhead() const;
    virtual void ApplySpeedControl();

};