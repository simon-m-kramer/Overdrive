// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboPIDController.h"

float FTurboPIDController::Update(float Error, float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return 0.0f;
	}

	// Proportional
	const float P = Kp * Error;

	// Integral (with anti-windup clamping)
	IntegralError += Error * DeltaTime;
	IntegralError = FMath::Clamp(IntegralError, -IntegralLimit, IntegralLimit);
	const float I = Ki * IntegralError;

	// Derivative (skip on first update — no previous error to compare against)
	float D = 0.0f;
	if (!bFirstUpdate)
	{
		const float DerivativeError = (Error - PreviousError) / DeltaTime;
		D = Kd * DerivativeError;
	}

	PreviousError = Error;
	bFirstUpdate = false;

	// Combine and clamp
	const float Output = P + I + D;
	return FMath::Clamp(Output, OutputMin, OutputMax);
}

void FTurboPIDController::Reset()
{
	IntegralError = 0.0f;
	PreviousError = 0.0f;
	bFirstUpdate = true;
}
