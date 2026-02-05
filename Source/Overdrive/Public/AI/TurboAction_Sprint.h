// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Sprint.generated.h"

UCLASS()
class OVERDRIVE_API UTurboAction_Sprint : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Sprint")
    float SpeedBoostKmh = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Sprint")
    float MinDistanceToCornerToExit = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Sprint")
    float MinDistanceToCarAhead = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Sprint")
    float SprintTimeout = 15.0f;

protected:
    virtual float FindTargetSpeedAhead() const override;

private:
    bool ShouldExit() const;

    float TimeInSprint = 0.0f;
    bool bSprintComplete = false;
};