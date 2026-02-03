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
    // RACING LINE CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float TrackWidth = 1200.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset", ClampMin = "0.0", ClampMax = "1.0"))
    float TrackWidthUsage = 0.85f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineLookahead = 10000.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineMinCurvature = 0.0001f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineSampleInterval = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float CurvatureSampleRange = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float CurvatureToOffsetScale = 2000000.0f;  // 1200000.0f

    // =========================================================================
    // RACING LINE ADVANCED
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    float ApproachSampleDistance = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    float LookaheadStepSize = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    float TurnSignLookahead = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    float CurvatureChangePercent = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    int32 SmoothingPasses = 5;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Advanced", meta = (EditCondition = "bUseRacingLineOffset"))
    int32 SmoothingWindow = 20;

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

    // Pre-calculated racing line
    TArray<float> PreCalculatedOffsets;
    bool bRacingLineCalculated = false;

    // =========================================================================
    // CORE METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    void UpdateSplineDistance();
    float GetLookaheadDistance() const;
    FVector GetTargetPoint();
    float CalculateSteering(const FVector& TargetPoint);

    // =========================================================================
    // RACING LINE
    // =========================================================================

    void PreCalculateRacingLine();
    float CalculateIdealOffset(float Distance) const;
    float GetPreCalculatedOffset(float Distance) const;
    float CalculateRacingLineOffset(float AtDistance) const;
    void DrawDebugRacingLine() const;
};