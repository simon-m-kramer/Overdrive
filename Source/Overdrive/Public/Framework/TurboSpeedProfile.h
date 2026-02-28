// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurboSpeedProfile.generated.h"

class ATurboRacingSpline;
class UTurboDrivingProfile;

USTRUCT(BlueprintType)
struct OVERDRIVE_API FTurboSpeedProfile
{
	GENERATED_BODY()

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile")
	float SampleInterval = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Profile")
	float CurvatureSampleRange = 400.0f;

	// =========================================================================
	// METHODS
	// =========================================================================

	void Calculate(const ATurboRacingSpline* Spline, const UTurboDrivingProfile* Profile);

	float GetTargetSpeed(float Distance) const;

	bool IsReady() const { return bReady; }

	void Reset();

private:
	TArray<float> Speeds;
	bool bReady = false;
	float CachedSplineLength = 0.0f;
	bool bCachedClosedLoop = false;
};