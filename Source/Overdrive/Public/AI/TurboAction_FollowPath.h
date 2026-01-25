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
    float MaxSpeedKmh = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float MinCornerSpeedKmh = 55.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float HairpinSpeedKmh = 40.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CornerDetectionDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureBrakingSensitivity = 130.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureSampleRange = 4000.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CoastingThreshold = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float ThrottleDeadzone = 10.0f;

    // =========================================================================
    // RACING LINE CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MaxRacingLineOffset = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineLookahead = 6000.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineMinCurvature = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float TrackWidthUsage = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineSmoothing = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MaxOffsetChangeRate = 400.0f;

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
    float SmoothedRacingLineOffset = 0.0f;
    float PreviousTargetOffset = 0.0f;

    // Pre-calculated racing line
    TArray<float> PreCalculatedOffsets;
    float RacingLineSampleInterval = 200.0f;
    bool bRacingLineCalculated = false;

    // =========================================================================
    // CORE METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    void UpdateSplineDistance();
    float GetLookaheadDistance() const;
    FVector GetTargetPoint(float DeltaTime);
    float CalculateSteering(const FVector& TargetPoint);

    // =========================================================================
    // SPEED CONTROL
    // =========================================================================

    float CalculateTargetSpeed() const;
    float FindMaxCurvatureAhead() const;
    void ApplySpeedControl();

    // =========================================================================
    // RACING LINE
    // =========================================================================

    void PreCalculateRacingLine();
    float CalculateIdealOffset(float Distance) const;
    float GetPreCalculatedOffset(float Distance) const;
    float CalculateRacingLineOffset(float AtDistance) const;
    void DrawDebugRacingLine() const;
};