// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboRacingLineCalculator.h"
#include "GameplayTagContainer.h"
#include "TurboRacingSpline.generated.h"

UCLASS()
class OVERDRIVE_API ATurboRacingSpline : public ATurboRacingLineCalculator
{
	GENERATED_BODY()

public:
	ATurboRacingSpline();

	/** Returns the racing line spline (used by AI for driving and position tracking) */
	USplineComponent* GetSplineComponent() const;

	const FGameplayTagContainer& GetGameplayTags() const { return GameplayTags; }

	// =========================================================================
	// SPLINE QUERIES
	// =========================================================================

	/** Get world-space location on the racing line at a given distance */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetLocationAtDistance(float Distance) const;

	/** Get world-space direction (tangent) at a given distance */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetDirectionAtDistance(float Distance) const;

	/** Get the total racing line length */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetSplineLength() const;

	/** Get whether the track is a closed loop */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	bool IsClosedLoop() const;

	/** Get full track width (derived from calculator's HalfTrackWidth) */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetTrackWidth() const { return HalfTrackWidth * 2.0f; }

	// =========================================================================
	// CURVATURE ANALYSIS
	// =========================================================================

	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetCurvatureAtDistance(float Distance, float SampleRange = 300.0f) const;

	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetTurnSign(float Distance, float InLookaheadDistance = 200.0f) const;

	float GetMaxTrackCurvature() const { return MaxTrackCurvature; }
	float GetMinCurvatureThreshold() const { return MinCurvatureThreshold; }

	// =========================================================================
	// CURVATURE CONFIGURATION
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Curvature")
	float CurvatureSampleRange = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Curvature", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float MinCurvatureThreshold = 0.05f;

	// =========================================================================
	// DEBUG
	// =========================================================================

	void DrawDebugRacingLine(UWorld* World) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer GameplayTags;

private:
	float WrapDistance(float Distance) const;
	void CalculateMaxCurvature();

	float MaxTrackCurvature = 0.0f;
};