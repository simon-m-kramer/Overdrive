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

    // Minimum speed for very sharp corners (curvature > 0.8)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float HairpinSpeedKmh = 40.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CornerDetectionDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureBrakingSensitivity = 130.0f;

    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CurvatureSampleRange = 4000.0f;

    // Speed threshold below which we coast instead of braking (km/h difference)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float CoastingThreshold = 20.0f;

    // Speed threshold below which we coast instead of accelerating (km/h difference)
    UPROPERTY(EditAnywhere, Category = "Speed", meta = (EditCondition = "bUseCurvatureSpeedControl"))
    float ThrottleDeadzone = 15.0f;

    // =========================================================================
    // STEERING CONFIGURATION
    // =========================================================================

    // Base steering multiplier
    UPROPERTY(EditAnywhere, Category = "Steering")
    float SteeringMultiplier = 2.0f;

    // Additional steering aggression at high speed when approaching corners
    UPROPERTY(EditAnywhere, Category = "Steering")
    float AggressiveSteeringMultiplier = 1.6f;

    // Curvature threshold to trigger aggressive steering
    UPROPERTY(EditAnywhere, Category = "Steering")
    float AggressiveSteeringCurvatureThreshold = 0.3f;

    // =========================================================================
    // RACING LINE CONFIGURATION
    // =========================================================================

    // Maximum lateral offset from centerline (cm)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MaxRacingLineOffset = 500.0f;

    // How far ahead to look for upcoming corners (cm)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineLookahead = 6000.0f;

    // Minimum curvature to trigger racing line behavior
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineMinCurvature = 0.1f;

    // How aggressively to use track width (1.0 = normal, 2.0 = very aggressive)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float TrackWidthUsage = 1.8f;

    // Minimum straight length to return to centerline (cm)
    // If the next corner is closer than this, stay on the racing line
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MinStraightForCenterline = 8000.0f;

    // How quickly the racing line offset changes (higher = more responsive, lower = smoother)
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float RacingLineSmoothing = 12.0f;

    // Maximum rate of offset change per second (cm/s) - prevents sudden jumps
    UPROPERTY(EditAnywhere, Category = "Racing Line", meta = (EditCondition = "bUseRacingLineOffset"))
    float MaxOffsetChangeRate = 1200.0f;

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

    // Smoothing state
    float SmoothedRacingLineOffset = 0.0f;
    float PreviousTargetOffset = 0.0f;

    float LockedApexDistance = -1.0f;
    float LockedTurnSign = 0.0f;
    bool bCornerLocked = false;

    // =========================================================================
    // METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    void UpdateSplineDistance();
    float GetLookaheadDistance() const;
    FVector GetTargetPoint(float DeltaTime);
    float CalculateSteering(const FVector& TargetPoint);

    // Speed control
    float CalculateTargetSpeed() const;
    float FindMaxCurvatureAhead() const;
    void ApplySpeedControl();

    // Racing line
    float CalculateRacingLineOffset(float AtDistance) const;

    // Corner detection helpers
    struct FCornerInfo
    {
        float ApexDistance = -1.0f;
        float Curvature = 0.0f;
        float TurnSign = 0.0f;
        bool bIsValid = false;
    };

    FCornerInfo FindNextCorner(float StartDistance, float SearchRange) const;
    FCornerInfo FindCornerAfterStraight(float StartDistance, float MaxSearchRange) const;

    // Pre-calculated racing line
    TArray<float> PreCalculatedOffsets;
    float RacingLineSampleInterval = 200.0f;
    bool bRacingLineCalculated = false;

    void PreCalculateRacingLine();
    float CalculateIdealOffset(float Distance) const;
    float GetPreCalculatedOffset(float Distance) const;
    void DrawDebugRacingLine() const;


};