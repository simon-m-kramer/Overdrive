// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "Components/TurboVehicleDetectionComponent.h"
#include "TurboAction_Overtake.generated.h"

class ATurboVehicle;

UCLASS()
class OVERDRIVE_API UTurboAction_Overtake : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    void Initialize(ATurboVehicle* InTargetVehicle, EOvertakeSide InSide);

    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float LateralOffset = 350.0f;

    UPROPERTY(EditAnywhere, Category = "Overtake")
    float OffsetBlendSpeed = 3.0f;

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