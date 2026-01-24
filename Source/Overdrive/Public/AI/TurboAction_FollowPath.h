// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BifrostAction.h"
#include "TurboAction_FollowPath.generated.h"

class ATurboAIController;
class ATurboVehicle;
class ATurboRacingSpline;
class USplineComponent;

UCLASS()
class OVERDRIVE_API UTurboAction_FollowPath : public UBifrostAction
{
    GENERATED_BODY()

public:
    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override { return false; }

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = true;

    // =========================================================================
    // FEATURE FLAGS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Features")
    bool bUseSpeedDependentLookahead = true;

    UPROPERTY(EditAnywhere, Category = "Features")
    bool bUseCurvatureSpeedControl = true;

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
    // SPEED CONFIGURATION
    // =========================================================================

    // Base target speed when driving straight (used when curvature control is off)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "!bUseCurvatureSpeedControl"))
    float TargetSpeedKmh = 80.0f;

    // Maximum speed on straights
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float MaxSpeedKmh = 220.0f;

    // Minimum speed in tightest corners
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float MinCornerSpeedKmh = 60.0f;

    // How far ahead to scan for corners (cm)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CornerDetectionDistance = 1500.0f;

    // How sensitive braking is to curvature (higher = more braking)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureBrakingSensitivity = 120.0f;

    // Sample range for curvature calculation (passed to GetCurvatureAtDistance)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureSampleRange = 4000.0f;

private:
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
    // STATE
    // =========================================================================

    float CurrentSplineDistance = 0.0f;

    // =========================================================================
    // METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    void UpdateSplineDistance();
    float GetLookaheadDistance() const;
    FVector GetTargetPoint() const;
    float CalculateSteering(const FVector& TargetPoint);

    // Speed control
    float CalculateTargetSpeed() const;
    float FindMaxCurvatureAhead() const;
    void ApplySpeedControl();
};
