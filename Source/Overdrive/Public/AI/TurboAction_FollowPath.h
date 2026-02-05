// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboActionBase.h"
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
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    // =========================================================================
    // FEATURE FLAGS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Features")
    bool bUseSpeedDependentLookahead = true;

    UPROPERTY(EditAnywhere, Category = "Features")
    bool bUseRacingLine = true;

    // =========================================================================
    // LOOKAHEAD CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Lookahead", meta = (EditCondition = "!bUseSpeedDependentLookahead"))
    float FixedLookaheadDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Lookahead", meta = (EditCondition = "bUseSpeedDependentLookahead"))
    float MinLookaheadDistance = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Lookahead", meta = (EditCondition = "bUseSpeedDependentLookahead"))
    float MaxLookaheadDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Lookahead", meta = (EditCondition = "bUseSpeedDependentLookahead"))
    float LookaheadSpeedFactor = 0.5f;

    // =========================================================================
    // SPEED CONTROL - LIMITS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float MaxSpeedKmh = 280.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float MinCornerSpeedKmh = 60.0f;

    // =========================================================================
    // SPEED CONTROL - PHYSICS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float GripFactor = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float SpeedCurvatureSampleRange = 1000.0f;

    // =========================================================================
    // SPEED CONTROL - CORNER SCANNING
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float CornerScanDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float CornerScanInterval = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Speed Control")
    float DistanceSpeedBuffer = 50.0f;

    // =========================================================================
    // SPEED CONTROL - INPUT TUNING
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
    TWeakObjectPtr<ATurboVehicle> Vehicle;

    UPROPERTY()
    TWeakObjectPtr<ATurboRacingSpline> RacingSplineActor;

    // =========================================================================
    // CORE METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    float GetLookaheadDistance() const;
    virtual FVector GetTargetPoint();
    float CalculateSteering(const FVector& TargetPoint);

    // =========================================================================
    // SPEED CONTROL
    // =========================================================================

    virtual float FindTargetSpeedAhead() const;
    virtual void ApplySpeedControl();
};