// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboLapTimingComponent.h"
#include "Framework/TurboRacingSpline.h"

UTurboLapTimingComponent::UTurboLapTimingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTurboLapTimingComponent::UpdateLapTiming(float DeltaTime, float CurrentSplineDistance, float PreviousSplineDistance)
{
	if (!RacingSpline || !RacingSpline->IsClosedLoop())
	{
		return;
	}

	const float SplineLength = RacingSpline->GetSplineLength();

	const bool bCrossedFinishLine = (PreviousSplineDistance > SplineLength * 0.9f) &&
		(CurrentSplineDistance < SplineLength * 0.1f);

	if (bCrossedFinishLine)
	{
		if (bLapTimingStarted)
		{
			LastLapTime = CurrentLapTime;

			if (BestLapTime <= 0.0f || CurrentLapTime < BestLapTime)
			{
				BestLapTime = CurrentLapTime;
			}

			LapCount++;
		}

		CurrentLapTime = 0.0f;
		bLapTimingStarted = true;
	}
	else if (bLapTimingStarted)
	{
		CurrentLapTime += DeltaTime;
	}
}

void UTurboLapTimingComponent::ResetLapTiming()
{
	CurrentLapTime = 0.0f;
	LastLapTime = 0.0f;
	BestLapTime = 0.0f;
	LapCount = 0;
	bLapTimingStarted = false;
}

FString UTurboLapTimingComponent::FormatLapTime(float TimeSeconds) const
{
	const int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	const float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
	return FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
}

