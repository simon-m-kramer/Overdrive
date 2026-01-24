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

    UPROPERTY(EditAnywhere, Category = "Features")
    bool bUseRacingLineOffset = true;

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

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "!bUseCurvatureSpeedControl"))
    float TargetSpeedKmh = 80.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float MaxSpeedKmh = 220.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float MinCornerSpeedKmh = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CornerDetectionDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureBrakingSensitivity = 120.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureSampleRange = 4000.0f;

    // =========================================================================
    // RACING LINE CONFIGURATION
    // =========================================================================

    // Maximum lateral offset from centerline (cm)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MaxRacingLineOffset = 400.0f;

    // How far ahead to look for upcoming corners (cm)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineLookahead = 6000.0f;

    // Minimum curvature to trigger racing line behavior
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineMinCurvature = 0.1f;

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

    // Racing line
    float CalculateRacingLineOffset(float AtDistance) const;
    float FindApexDistance(float StartDistance, float SearchRange) const;
};
