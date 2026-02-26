// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurboRaceManager.generated.h"

class ATurboVehicle;
class ATurboRacingSpline;
class USplineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleFinished, ATurboVehicle*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownUpdated, int32, Count);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);

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

	UPROPERTY(BlueprintReadOnly)
	float CurrentLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float LastLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	float PreviousSplineDistance = 0.0f;
	bool bLapTimingStarted = false;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVERDRIVE_API UTurboRaceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurboRaceManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Race")
	FOnVehicleFinished OnVehicleFinished;

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Race")
	int32 TotalLaps = 3;

	// =========================================================================
	// QUERIES
	// =========================================================================

	UFUNCTION(BlueprintCallable, Category = "Race")
	int32 GetPlacement(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	int32 GetCurrentLap(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	const TArray<FRaceEntry>& GetStandings() const { return Entries; }

	UFUNCTION(BlueprintCallable, Category = "Race")
	int32 GetEntryCount() const { return Entries.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Race")
	bool HasFinished(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	float GetCurrentLapTime(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	float GetLastLapTime(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	float GetBestLapTime(ATurboVehicle* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Race")
	ATurboRacingSpline* GetRacingSpline() const { return RacingSpline; }

	UFUNCTION(BlueprintPure, Category = "Race")
	static FString FormatLapTime(float TimeSeconds);

	// =========================================================================
	// COUNTDOWN
	// =========================================================================

	UFUNCTION(BlueprintCallable, Category = "Race")
	void StartCountdown();

	UFUNCTION(BlueprintPure, Category = "Race")
	bool HasRaceStarted() const { return bRaceStarted; }

	UPROPERTY(BlueprintAssignable, Category = "Race")
	FOnCountdownUpdated OnCountdownUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Race")
	FOnRaceStarted OnRaceStarted;

	UPROPERTY(EditAnywhere, Category = "Race")
	int32 CountdownSeconds = 2;

	// =========================================================================
	// DEBUG
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Race|Debug")
	bool bDrawDebug = false;

private:
	void FindRacingSpline();
	void CollectVehicles();
	void UpdateProgress(float DeltaTime);
	void DetectLapCompletion(FRaceEntry& Entry, float SplineLength);
	void SortStandings();

	UPROPERTY()
	TObjectPtr<ATurboRacingSpline> RacingSpline;

	UPROPERTY()
	TArray<FRaceEntry> Entries;

	float SplineLength = 0.0f;

	// Countdown
	void UpdateCountdown();
	FTimerHandle CountdownTimerHandle;
	int32 CurrentCount = 0;
	bool bRaceStarted = false;
};
