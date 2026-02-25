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
	float CurvatureSampleRange = 400.0f;

	/** Safety margin — multiplier on cornering speed (< 1.0 = cautious, > 1.0 = aggressive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float CorneringSpeedSafetyFactor = 1.0;

	/** Acceleration boost when exiting corners (curvature decreasing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float ExitAccelerationBoost = 2.0f;

	// =========================================================================
	// METHODS
	// =========================================================================

	void Calculate(const ATurboRacingSpline* Spline, const ATurboVehicle* Vehicle);

	float GetTargetSpeed(float Distance, float SplineLength, bool bClosedLoop) const;

	bool IsReady() const { return bReady; }

	void Reset();

private:
	TArray<float> Speeds;
	bool bReady = false;
};