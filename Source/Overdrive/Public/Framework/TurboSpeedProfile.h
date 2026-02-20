// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurboSpeedProfile.generated.h"

class ATurboRacingSpline;
class ATurboVehicle;

USTRUCT(BlueprintType)
struct OVERDRIVE_API FTurboSpeedProfile
{
	GENERATED_BODY()

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	/** Distance between speed profile sample points (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile")
	float SampleInterval = 100.0f;

	/** Curvature sample range used for speed calculations (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile")
	float CurvatureSampleRange = 100.0f;

	/** Safety margin — multiplier on cornering speed (< 1.0 = cautious, > 1.0 = aggressive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float CorneringSpeedSafetyFactor = 1.0f;

	/** Acceleration boost when exiting corners (curvature decreasing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float ExitAccelerationBoost = 1.0f;

	// =========================================================================
	// METHODS
	// =========================================================================

	/**
	 * Calculate speed targets from spline curvature and vehicle performance.
	 * Three-pass algorithm:
	 *   Pass 1: Cornering speed limits from curvature
	 *   Pass 2: Braking constraints (reverse propagation)
	 *   Pass 3: Acceleration constraints (forward propagation, with exit boost)
	 *
	 * @param Spline   The racing line to analyze
	 * @param Vehicle  The vehicle whose performance stats to use
	 */
	void Calculate(const ATurboRacingSpline* Spline, const ATurboVehicle* Vehicle);

	/**
	 * Get interpolated target speed at a given distance along the spline.
	 *
	 * @param Distance     Distance along the spline (cm)
	 * @param SplineLength Total spline length for wrapping (cm)
	 * @param bClosedLoop  Whether the spline is a closed loop
	 * @return             Target speed in cm/s
	 */
	float GetTargetSpeed(float Distance, float SplineLength, bool bClosedLoop) const;

	/** Whether the profile has been successfully calculated */
	bool IsReady() const { return bReady; }

	/** Reset the profile (clears all data) */
	void Reset();

private:
	TArray<float> Speeds;
	bool bReady = false;
};