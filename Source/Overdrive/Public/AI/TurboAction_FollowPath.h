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
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

    UPROPERTY(EditAnywhere, Category = "Lookahead")
    float LookaheadDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Speed")
    float TargetSpeedKmh = 60.0f;

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

    // =========================================================================
    // METHODS
    // =========================================================================

    USplineComponent* GetSpline() const;
    void UpdateSplineDistance();
    FVector GetTargetPoint() const;
    float CalculateSteering(const FVector& TargetPoint);
    void ApplySimpleSpeedControl();
};
