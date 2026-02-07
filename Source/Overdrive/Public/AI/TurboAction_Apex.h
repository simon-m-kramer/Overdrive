// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Apex.generated.h"

UENUM(BlueprintType)
enum class EApexCornerPhase : uint8
{
    Straight,
    Approach,
    TrailBrake,
    Apex,
    Exit
};

UCLASS()
class OVERDRIVE_API UTurboAction_Apex : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    UTurboAction_Apex();

    virtual void Update(float DeltaTime) override;

    // =========================================================================
    // TRAIL BRAKING
    // =========================================================================

    /** How much steering input before trail braking engages (0-1) */
    UPROPERTY(EditAnywhere, Category = "Apex|Trail Braking")
    float TrailBrakeSteeringThreshold = 0.05f;

    /** Max brake input during trail braking phase */
    UPROPERTY(EditAnywhere, Category = "Apex|Trail Braking")
    float TrailBrakeMaxBrake = 0.7f;

    /** How aggressively brake releases as steering increases (higher = faster release) */
    UPROPERTY(EditAnywhere, Category = "Apex|Trail Braking")
    float TrailBrakeReleaseFactor = 1.5f;

    /** Throttle to apply during trail braking (slight maintenance throttle) */
    UPROPERTY(EditAnywhere, Category = "Apex|Trail Braking")
    float TrailBrakeThrottle = 0.0f;

    /** How quickly steering adjusts (lower = smoother, higher = more responsive) */
    UPROPERTY(EditAnywhere, Category = "Apex|Steering")
    float SteeringInterpSpeed = 50.0f;

    // =========================================================================
    // CORNER EXIT
    // =========================================================================

    /** How quickly to ramp throttle on corner exit (higher = more aggressive) */
    UPROPERTY(EditAnywhere, Category = "Apex|Exit")
    float ExitThrottleRampRate = 4.0f;

    /** Minimum throttle on corner exit even while still turning */
    UPROPERTY(EditAnywhere, Category = "Apex|Exit")
    float ExitMinThrottle = 0.3f;

    // =========================================================================
    // AGGRESSIVE SCANNING
    // =========================================================================

    /** Shorter scan distance — brake later */
    UPROPERTY(EditAnywhere, Category = "Apex|Speed")
    float ApexCornerScanDistance = 3000.0f;

    /** Bigger buffer — carry more speed toward corners */
    UPROPERTY(EditAnywhere, Category = "Apex|Speed")
    float ApexDistanceSpeedBuffer = 120.0f;

    /** Lower grip factor — more conservative corner speed estimate to compensate for late braking */
    UPROPERTY(EditAnywhere, Category = "Apex|Speed")
    float ApexGripFactor = 1600.0f;

    // =========================================================================
    // PHASE DETECTION
    // =========================================================================

    /** Curvature above this = in a corner */
    UPROPERTY(EditAnywhere, Category = "Apex|Phase")
    float CornerCurvatureThreshold = 0.0003f;

    /** Scan distance ahead for detecting upcoming corners */
    UPROPERTY(EditAnywhere, Category = "Apex|Phase")
    float ApproachScanDistance = 2000.0f;

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Apex|Debug")
    bool bDrawApexDebug = true;

protected:
    virtual void ApplySpeedControl() override;
    virtual float FindTargetSpeedAhead() const override;

private:
    EApexCornerPhase DetermineCornerPhase() const;
    float GetCurrentCurvature() const;
    float GetUpcomingCurvature() const;
    bool IsCurvatureIncreasing() const;

    float CurrentSteeringInput = 0.0f;
    float ExitThrottleAlpha = 0.0f;
    EApexCornerPhase CurrentPhase = EApexCornerPhase::Straight;
    bool IsCurvatureDecreasing() const;
    float SmoothedSteeringInput = 0.0f;

};
