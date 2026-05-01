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

	/** Proportional gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Kp = 1.0f;

	/** Integral gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Ki = 0.0f;

	/** Derivative gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PID")
	float Kd = 0.0f;

	// =========================================================================
	// LIMITS
	// =========================================================================

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

	float Update(float Error, float DeltaTime);

	void Reset();

private:
	float IntegralError = 0.0f;
	float PreviousError = 0.0f;
	bool bFirstUpdate = true;
};