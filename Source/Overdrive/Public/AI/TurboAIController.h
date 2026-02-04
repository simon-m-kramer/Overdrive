// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TurboAIController.generated.h"

class UBifrostActionStack;
class UBifrostAction;
class ATurboRacingSpline;
class ATurboVehicle;

UCLASS()
class OVERDRIVE_API ATurboAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATurboAIController();

    virtual void Tick(float DeltaTime) override;

    // =========================================================================
    // ACTION STACK
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Action")
    void PushAction(UBifrostAction* NewAction);

    UFUNCTION(BlueprintCallable, Category = "Action")
    void RemoveAction(UBifrostAction* InAction);

    UFUNCTION(BlueprintPure, Category = "Action")
    bool Contains(UBifrostAction* InAction);

    UFUNCTION(BlueprintPure, Category = "Action")
    bool IsEmpty() const;

    UFUNCTION(BlueprintPure, Category = "Action")
    UBifrostAction* GetCurrentAction() const;

    // =========================================================================
    // SPLINE & POSITION
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Spline")
    void FindRacingSpline();

    UFUNCTION(BlueprintPure, Category = "Spline")
    ATurboRacingSpline* GetRacingSplineActor() const { return RacingSplineActor; }

    UFUNCTION(BlueprintPure, Category = "Spline")
    float GetCurrentSplineDistance() const { return CurrentSplineDistance; }

    UFUNCTION(BlueprintCallable, Category = "Spline")
    void UpdateSplineDistance();

    // =========================================================================
    // VEHICLE
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    ATurboVehicle* GetControlledVehicle() const { return ControlledVehicle; }

    // =========================================================================
    // LAP TIMING
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Lap Timing")
    float GetCurrentLapTime() const { return CurrentLapTime; }

    UFUNCTION(BlueprintPure, Category = "Lap Timing")
    float GetLastLapTime() const { return LastLapTime; }

    UFUNCTION(BlueprintPure, Category = "Lap Timing")
    float GetBestLapTime() const { return BestLapTime; }

    UFUNCTION(BlueprintPure, Category = "Lap Timing")
    int32 GetLapCount() const { return LapCount; }

    UFUNCTION(BlueprintCallable, Category = "Lap Timing")
    void ResetLapTiming();

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = true;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowLapTiming = true;

    // =========================================================================
    // Performance
    // =========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceMultiplier = 1.0f;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
    TObjectPtr<UBifrostActionStack> ActionStack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    TObjectPtr<ATurboRacingSpline> RacingSplineActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    TObjectPtr<ATurboVehicle> ControlledVehicle;

private:
    void UpdateLapTiming(float DeltaTime);
    FString FormatLapTime(float TimeSeconds) const;

    float CurrentSplineDistance = 0.0f;
    float PreviousSplineDistance = 0.0f;

    // Lap timing
    float CurrentLapTime = 0.0f;
    float LastLapTime = 0.0f;
    float BestLapTime = 0.0f;
    int32 LapCount = 0;
    bool bLapTimingStarted = false;
};