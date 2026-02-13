// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurboPIDController.generated.h"

USTRUCT(BlueprintType)
struct OVERDRIVE_API FTurboPIDController
{
	GENERATED_BODY()

	// =========================================================================
	// GAINS
	// =========================================================================

	/** Proportional gain — reacts to current error */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Kp = 1.0f;

	/** Integral gain — reacts to accumulated error over time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Ki = 0.0f;

	/** Derivative gain — reacts to rate of change of error */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Kd = 0.0f;

	// =========================================================================
	// LIMITS
	// =========================================================================

	/** Clamp output to this range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float OutputMin = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float OutputMax = 1.0f;

	/** Clamp integral accumulation to prevent windup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float IntegralLimit = 100.0f;

	// =========================================================================
	// METHODS
	// =========================================================================

	/**
	 * Compute PID output from current error.
	 * Call once per tick with the current error value and delta time.
	 *
	 * @param Error		The current error (target - actual)
	 * @param DeltaTime	Time since last update in seconds
	 * @return			Control output, clamped to [OutputMin, OutputMax]
	 */
	float Update(float Error, float DeltaTime);

	/** Reset all accumulated state (call when action starts/resumes) */
	void Reset();

private:
	float IntegralError = 0.0f;
	float PreviousError = 0.0f;
	bool bFirstUpdate = true;
};