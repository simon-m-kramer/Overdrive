// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "GameplayTagContainer.h"
#include "TurboAIController.generated.h"

class ATurboRacingSpline;
class ATurboAIVehicle;
class UTurboActionStack;
class UTurboActionBase;
class UTurboAction_FollowPath;

USTRUCT(BlueprintType)
struct FTurboDecisionContext
{
    GENERATED_BODY()
    // TO DO: Add decision making properties
};

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

    UFUNCTION(BlueprintPure, Category = "Action")
    const TArray<UBifrostAction*>& GetActions() const;

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
    ATurboAIVehicle* GetControlledVehicle() const { return ControlledVehicle; }

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
    // DECISION MAKING
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Decision Making")
    const FTurboDecisionContext& GetDecisionContext() const { return DecisionContext; }

    UPROPERTY(EditAnywhere, Category = "Decision Making")
    float StraightCurvatureThreshold = 0.0002f;

    UPROPERTY(EditAnywhere, Category = "Decision Making")
    float CornerScanDistance = 3000.0f;

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = true;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowLapTiming = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDecisionContext = false;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
    TSubclassOf<UTurboAction_FollowPath> DefaultActionClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
    TArray<TSubclassOf<UTurboActionBase>> ActionPriorityList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
    FGameplayTagContainer DisabledActions;

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
    TObjectPtr<UTurboActionStack> ActionStack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    TObjectPtr<ATurboRacingSpline> RacingSplineActor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    TObjectPtr<ATurboAIVehicle> ControlledVehicle;

private:
    // Decision making
    FTurboDecisionContext DecisionContext;
    void UpdateDecisionContext();
    float FindDistanceToNextCorner() const;

    // Spline
    float CurrentSplineDistance = 0.0f;
    float PreviousSplineDistance = 0.0f;

    // Lap timing
    void UpdateLapTiming(float DeltaTime);
    FString FormatLapTime(float TimeSeconds) const;
    float CurrentLapTime = 0.0f;
    float LastLapTime = 0.0f;
    float BestLapTime = 0.0f;
    int32 LapCount = 0;
    bool bLapTimingStarted = false;


};