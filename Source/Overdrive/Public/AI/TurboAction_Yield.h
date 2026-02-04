// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Yield.generated.h"

UCLASS()
class OVERDRIVE_API UTurboAction_Yield : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Yield")
    float SpeedReductionKmh = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Yield")
    float YieldTimeout = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Yield")
    float ClearDuration = 1.0f;

protected:
    virtual float FindTargetSpeedAhead() const override;

private:
    bool IsCarBeside() const;

    float TimeInYield = 0.0f;
    float TimeSinceClear = 0.0f;
    bool bYieldComplete = false;
};