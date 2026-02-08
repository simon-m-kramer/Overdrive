// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Overtake.generated.h"

class ATurboVehicle;
struct FTurboDecisionContext;

UCLASS()
class OVERDRIVE_API UTurboAction_Overtake : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    UTurboAction_Overtake();

    virtual bool CanActivate(const FTurboDecisionContext& Context) const override;
    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

    // =========================================================================
    // ACTIVATION CONDITIONS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Overtake|Activation")
    float ConsiderDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake|Activation")
    float MinSpeedAdvantage = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake|Activation")
    float OvertakeMaxCurvature = 0.0001f;

    // =========================================================================
    // BEHAVIOR
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float MinDistanceAheadToComplete = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float AbortTimeout = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float SpeedBoostKmh = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float CompletionHoldTime = 3.0f;

    /** How fast the AI blends between primary and secondary line (0-1 per second) */
    UPROPERTY(EditAnywhere, Category = "Overtake")
    float LaneBlendSpeed = 2.0f;

protected:
    virtual FVector GetTargetPoint() override;
    virtual float FindTargetSpeedAhead() const override;

private:
    bool HasPassedTargetVehicle() const;
    bool ShouldAbort() const;

    UPROPERTY()
    TWeakObjectPtr<ATurboVehicle> TargetVehicle;

    /** 0 = primary line, 1 = secondary line */
    float LaneBlendAlpha = 0.0f;

    float TimeInOvertake = 0.0f;
    bool bOvertakeComplete = false;
    bool bHasPassedTarget = false;
    float TimeSincePassed = 0.0f;
};