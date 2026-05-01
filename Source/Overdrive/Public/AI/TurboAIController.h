// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "GameplayTagContainer.h"
#include "TurboAIController.generated.h"

class ATurboRacingSpline;
class ATurboVehicle;
class UTurboActionStack;
class UTurboActionBase;
class UTurboAction_FollowPath;

USTRUCT(BlueprintType)
struct FTurboDecisionContext
{
    GENERATED_BODY()

    // =========================================================================
    // DETECTION
    // =========================================================================

    bool bVehicleAhead = false;
    float DistanceToVehicleAhead = 0.0f;
    float SpeedOfVehicleAheadCms = 0.0f;
    float SpeedDifferenceCms = 0.0f;

    bool bVehicleOnLeft = false;
    bool bVehicleOnRight = false;
    bool bVehicleBehind = false;

    // =========================================================================
    // VEHICLE STATE
    // =========================================================================

    float CurrentSpeedCms = 0.0f;
    float CurrentSplineDistance = 0.0f;
    float TargetSpeedCms = 0.0f;

    // =========================================================================
    // TRACK ANALYSIS
    // =========================================================================

    float DistanceToNextCorner = 0.0f;
    float CurrentCurvature = 0.0f;

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
    // SPLINE
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Spline")
    void FindRacingSpline();

    UFUNCTION(BlueprintCallable, Category = "Spline")
    void UpdateSplineDistance();

    UFUNCTION(BlueprintPure, Category = "Spline")
    ATurboRacingSpline* GetRacingSplineActor() const { return RacingSpline; }    // TODO: Rename this to GetRacingSpline

    UFUNCTION(BlueprintPure, Category = "Spline")
    float GetCurrentSplineDistance() const { return CurrentSplineDistance; }

    // =========================================================================
    // VEHICLE
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    ATurboVehicle* GetVehicle() const { return Vehicle; }

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    UTurboDrivingProfile* GetDrivingProfile() const { return DrivingProfile; }

    // =========================================================================
    // DECISION MAKING
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Decision Making")
    const FTurboDecisionContext& GetDecisionContext() const { return DecisionContext; }

    UPROPERTY(EditAnywhere, Category = "Track Analysis")
    float CornerCurvatureThreshold = 0.0001f;  // radians/cm, this value needs calibrated properly according to the scale of the track

    UPROPERTY(EditAnywhere, Category = "Track Analysis")
    float CornerScanStep = 200.0f;  // cm

    UPROPERTY(EditAnywhere, Category = "Track Analysis")
    float CornerScanMax = 10000.0f;  // cm

    UPROPERTY(EditAnywhere, Category = "Track Analysis")
    float CornerScanSampleRange = 400.0f;  // cm

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
    TSubclassOf<UTurboAction_FollowPath> DefaultActionClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Personality")
    TArray<TSubclassOf<UTurboActionBase>> ActionPriorityList;

    UPROPERTY(EditAnywhere, Category = "AI Personality")
    TObjectPtr<UTurboDrivingProfile> DrivingProfile;

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
    TObjectPtr<UTurboActionStack> ActionStack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    TObjectPtr<ATurboRacingSpline> RacingSpline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    TObjectPtr<ATurboVehicle> Vehicle;

private:
    // Decision making
    FTurboDecisionContext DecisionContext;
    void UpdateDecisionContext();

    // Spline
    float CurrentSplineDistance = 0.0f;

    // Initialization helpers (called from OnPossess)
    void InitializeSplineDistance();
    void InitializeActionStack();
    void PopulateActionRoster();
    void PushDefaultAction();
    void PushGridStartAction();

};