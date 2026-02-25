// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboSpeedProfile.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboDrivingProfile.h"

void FTurboSpeedProfile::Calculate(const ATurboRacingSpline* Spline, const UTurboDrivingProfile* Profile)
{
	bReady = false;
	Speeds.Empty();

	if (!Spline || !Profile) return;

	const float SplineLength = Spline->GetSplineLength();
	const int32 NumSamples = FMath::CeilToInt(SplineLength / SampleInterval);

	if (NumSamples <= 0) return;

	const float MaxSpeed = Profile->MaxSpeedCms;
	const float Grip = Profile->LateralGripCms2;
	const float BrakeDecel = Profile->BrakeDecelerationCms2;
	const float Accel = Profile->AccelerationCms2;
	const float CorneringSpeedSafetyFactor = Profile->CorneringSpeedSafetyFactor;
	const float ExitAccelerationBoost = Profile->ExitAccelerationBoost;

	Speeds.SetNum(NumSamples);

	// ---- Pass 1: Cornering speed limits ----
	// v = sqrt(grip / curvature)


	for (int32 i = 0; i < NumSamples; i++)
	{
		const float Dist = i * SampleInterval;
		const float Curvature = Spline->GetCurvatureAtDistance(Dist, CurvatureSampleRange);

		if (Curvature > KINDA_SMALL_NUMBER)
		{
			const float CorneringSpeed = FMath::Sqrt(Grip / Curvature) * CorneringSpeedSafetyFactor;
			Speeds[i] = FMath::Min(MaxSpeed, CorneringSpeed);
		}
		else
		{
			Speeds[i] = MaxSpeed;
		}
	}

	// ---- Pass 2: Braking pass ----
	// v = sqrt(v_next² + 2 * brakeDecel * distance)

	const bool bClosedLoop = Spline->IsClosedLoop();
	const float Ds = SampleInterval;  // Ds = delta s

	if (bClosedLoop)
	{
		for (int32 Lap = 0; Lap < 2; Lap++)
		{
			for (int32 i = NumSamples - 1; i >= 0; i--)
			{
				const int32 NextIndex = (i + 1) % NumSamples;
				const float BrakeLimit = FMath::Sqrt(Speeds[NextIndex] * Speeds[NextIndex] + 2.0f * BrakeDecel * Ds);
				Speeds[i] = FMath::Min(Speeds[i], BrakeLimit);
			}
		}
	}
	else
	{
		for (int32 i = NumSamples - 2; i >= 0; i--)
		{
			const float BrakeLimit = FMath::Sqrt(Speeds[i + 1] * Speeds[i + 1] + 2.0f * BrakeDecel * Ds);
			Speeds[i] = FMath::Min(Speeds[i], BrakeLimit);
		}
	}

	// ---- Pass 3: Acceleration pass ----
	// v = sqrt(v_prev² + 2 * accel * distance)

	if (bClosedLoop)
	{
		for (int32 Lap = 0; Lap < 2; Lap++)
		{
			for (int32 i = 0; i < NumSamples; i++)
			{
				const int32 PrevIndex = (i - 1 + NumSamples) % NumSamples;

				float EffectiveAccel = Accel;
				if (ExitAccelerationBoost > 1.0f)
				{
					const float CurvHere = Spline->GetCurvatureAtDistance(i * SampleInterval, CurvatureSampleRange);
					const float CurvPrev = Spline->GetCurvatureAtDistance(PrevIndex * SampleInterval, CurvatureSampleRange);

					if (CurvHere < CurvPrev)
					{
						EffectiveAccel *= ExitAccelerationBoost;
					}
				}

				const float AccelLimit = FMath::Sqrt(Speeds[PrevIndex] * Speeds[PrevIndex] + 2.0f * EffectiveAccel * Ds);
				Speeds[i] = FMath::Min(Speeds[i], AccelLimit);
			}
		}
	}
	else
	{
		for (int32 i = 1; i < NumSamples; i++)
		{
			float EffectiveAccel = Accel;
			if (ExitAccelerationBoost > 1.0f)
			{
				const float CurvHere = Spline->GetCurvatureAtDistance(i * SampleInterval, CurvatureSampleRange);
				const float CurvPrev = Spline->GetCurvatureAtDistance((i - 1) * SampleInterval, CurvatureSampleRange);

				if (CurvHere < CurvPrev)
				{
					EffectiveAccel *= ExitAccelerationBoost;
				}
			}

			const float AccelLimit = FMath::Sqrt(Speeds[i - 1] * Speeds[i - 1] + 2.0f * EffectiveAccel * Ds);
			Speeds[i] = FMath::Min(Speeds[i], AccelLimit);
		}
	}

	bReady = true;
}

float FTurboSpeedProfile::GetTargetSpeed(float Distance, float SplineLength, bool bClosedLoop) const
{
	if (!bReady || Speeds.Num() == 0) return 0.0f;

	// wrap around
	if (bClosedLoop)
	{
		Distance = FMath::Fmod(Distance, SplineLength);
		if (Distance < 0.0f) Distance += SplineLength;
	}
	else
	{
		Distance = FMath::Clamp(Distance, 0.0f, SplineLength - KINDA_SMALL_NUMBER);
	}

	// interpolate
	const float IndexFloat = Distance / SampleInterval;
	const int32 Index = FMath::Clamp(FMath::FloorToInt(IndexFloat), 0, Speeds.Num() - 1);
	const int32 NextIndex = (Index + 1) % Speeds.Num();
	const float Alpha = IndexFloat - FMath::FloorToInt(IndexFloat);

	return FMath::Lerp(Speeds[Index], Speeds[NextIndex], Alpha);
}

void FTurboSpeedProfile::Reset()
{
	Speeds.Empty();
	bReady = false;
}