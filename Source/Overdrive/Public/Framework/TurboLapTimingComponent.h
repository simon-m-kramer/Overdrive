// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurboLapTimingComponent.generated.h"

class ATurboRacingSpline;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVERDRIVE_API UTurboLapTimingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurboLapTimingComponent();

	void SetRacingSpline(ATurboRacingSpline* InSpline) { RacingSpline = InSpline; }

	/** Call every tick with the vehicle's current spline distance */
	void UpdateLapTiming(float DeltaTime, float CurrentSplineDistance, float PreviousSplineDistance);

	UFUNCTION(BlueprintCallable, Category = "Lap Timing")
	void ResetLapTiming();

	UFUNCTION(BlueprintPure, Category = "Lap Timing")
	float GetCurrentLapTime() const { return CurrentLapTime; }

	UFUNCTION(BlueprintPure, Category = "Lap Timing")
	float GetLastLapTime() const { return LastLapTime; }

	UFUNCTION(BlueprintPure, Category = "Lap Timing")
	float GetBestLapTime() const { return BestLapTime; }

	UFUNCTION(BlueprintPure, Category = "Lap Timing")
	int32 GetLapCount() const { return LapCount; }

	UFUNCTION(BlueprintPure, Category = "Lap Timing")
	FString FormatLapTime(float TimeSeconds) const;

private:
	UPROPERTY()
	TObjectPtr<ATurboRacingSpline> RacingSpline;

	float CurrentLapTime = 0.0f;
	float LastLapTime = 0.0f;
	float BestLapTime = 0.0f;
	int32 LapCount = 0;
	bool bLapTimingStarted = false;
};
