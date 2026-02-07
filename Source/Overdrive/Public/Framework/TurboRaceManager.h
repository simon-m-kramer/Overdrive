// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurboRaceManager.generated.h"

class ATurboVehicle;
class ATurboRacingSpline;
class USplineComponent;

USTRUCT(BlueprintType)
struct FRaceEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<ATurboVehicle> Vehicle;

    UPROPERTY(BlueprintReadOnly)
    int32 Placement = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentLap = 0;

    UPROPERTY(BlueprintReadOnly)
    float SplineDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TotalProgress = 0.0f;

    /** Previous frame's spline distance for lap detection */
    float PreviousSplineDistance = 0.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVERDRIVE_API UTurboRaceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UTurboRaceManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Race")
    int32 TotalLaps = 3;

    // =========================================================================
    // QUERIES
    // =========================================================================

    /** Get placement for a specific vehicle (1 = first place) */
    UFUNCTION(BlueprintCallable, Category = "Race")
    int32 GetPlacement(ATurboVehicle* Vehicle) const;

    /** Get current lap for a specific vehicle */
    UFUNCTION(BlueprintCallable, Category = "Race")
    int32 GetCurrentLap(ATurboVehicle* Vehicle) const;

    /** Get the full sorted standings */
    UFUNCTION(BlueprintCallable, Category = "Race")
    const TArray<FRaceEntry>& GetStandings() const { return Entries; }

    /** Get entry count */
    UFUNCTION(BlueprintCallable, Category = "Race")
    int32 GetEntryCount() const { return Entries.Num(); }

    /** Check if a vehicle has finished the race */
    UFUNCTION(BlueprintCallable, Category = "Race")
    bool HasFinished(ATurboVehicle* Vehicle) const;

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Race|Debug")
    bool bDrawDebug = true;

private:
    void CollectVehicles();
    void UpdateProgress();
    void DetectLapCompletion(FRaceEntry& Entry, float SplineLength);
    void SortStandings();

    UPROPERTY()
    TObjectPtr<ATurboRacingSpline> RacingSpline;

    UPROPERTY()
    TArray<FRaceEntry> Entries;

    float SplineLength = 0.0f;
};
