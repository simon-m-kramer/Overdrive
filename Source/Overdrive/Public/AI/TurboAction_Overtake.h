// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "Components/TurboVehicleDetectionComponent.h"
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
    float OvertakeMaxCurvature = 0.0005f;

    // =========================================================================
    // BEHAVIOR
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float LateralOffset = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float OffsetBlendSpeed = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float MinDistanceAheadToComplete = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float AbortTimeout = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float SpeedBoostKmh = 15.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float CompletionHoldTime = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float CornerAbortCurvature = 0.001f;



protected:
    virtual FVector GetTargetPoint() override;
    virtual float FindTargetSpeedAhead() const override;

private:
    EOvertakeSide ChooseOvertakeSide(const FTurboDecisionContext& Context) const;
    bool HasPassedTargetVehicle() const;
    bool ShouldAbort() const;

    UPROPERTY()
    TWeakObjectPtr<ATurboVehicle> TargetVehicle;

    EOvertakeSide Side = EOvertakeSide::Left;

    float CurrentLateralOffset = 0.0f;
    float TargetSplineDistanceAtStart = 0.0f;
    float TimeInOvertake = 0.0f;
    bool bOvertakeComplete = false;
    bool bHasPassedTarget = false;
    float TimeSincePassed = 0.0f;
};