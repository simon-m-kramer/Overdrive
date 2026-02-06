// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/TurboAction_FollowPath.h"
#include "TurboAction_Evade.generated.h"

struct FTurboDecisionContext;

UCLASS()
class OVERDRIVE_API UTurboAction_Evade : public UTurboAction_FollowPath
{
    GENERATED_BODY()

public:
    UTurboAction_Evade();

    virtual bool CanActivate(const FTurboDecisionContext& Context) const override;
    virtual void Start(bool bFirstTime) override;
    virtual void Update(float DeltaTime) override;
    virtual bool IsDone() override;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Evade")
    float LateralOffset = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Evade")
    float OffsetBlendSpeed = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Evade")
    float EvadeTimeout = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Evade")
    float ClearDuration = 0.5f;

protected:
    virtual FVector GetTargetPoint() override;

private:
    bool IsCarBeside() const;
    bool IsCarOnLeft() const;
    bool IsCarOnRight() const;

    float CurrentLateralOffset = 0.0f;
    float TimeInEvade = 0.0f;
    float TimeSinceClear = 0.0f;
    bool bEvadeComplete = false;
    bool bEvadingLeft = false;  // True = car is on left, evading right
};
