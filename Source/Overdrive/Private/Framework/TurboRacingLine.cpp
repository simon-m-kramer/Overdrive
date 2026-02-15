// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingLine.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"

UTurboRacingLine::UTurboRacingLine()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true; // Only enable if debug drawing is needed.
}

// -----------------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------------

void UTurboRacingLine::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateRacingLine();
	}

	if (bDrawDebugLine)
	{
		PrimaryComponentTick.bStartWithTickEnabled = true;
		SetComponentTickEnabled(true);
	}
}

void UTurboRacingLine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDrawDebugLine)
	{
		DrawDebug();
	}
}

// -----------------------------------------------------------------------------
// Generation
// -----------------------------------------------------------------------------

TArray<FVector> UTurboRacingLine::SampleCenterline() const
{
	TArray<FVector> Points;

	if (!CenterlineSpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurboRacingLine: CenterlineSpline is null."));
		return Points;
	}

	const float SplineLength = CenterlineSpline->GetSplineLength();
	const int32 Count = Optimizer.NumSamples;
	const float Step = SplineLength / static_cast<float>(Count);

	Points.Reserve(Count);
	for (int32 I = 0; I < Count; ++I)
	{
		const float Distance = I * Step;
		Points.Add(CenterlineSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World));
	}

	return Points;
}

void UTurboRacingLine::GenerateRacingLine()
{
	const TArray<FVector> CenterlinePoints = SampleCenterline();
	if (CenterlinePoints.Num() < 3)
	{
		return;
	}

	// Push settings into the optimizer and run.
	Optimizer.TrackWidth = TrackWidth;
	Optimizer.Optimize(CenterlinePoints);

	// Cache the result locally.
	RacingLinePoints = Optimizer.GetRacingLine();
	BuildDistanceLUT();

	UE_LOG(LogTemp, Log, TEXT("UTurboRacingLine: Generated racing line with %d points (%.0f cm total)."),
		RacingLinePoints.Num(), GetTotalDistance());
}

// -----------------------------------------------------------------------------
// Distance LUT
// -----------------------------------------------------------------------------

void UTurboRacingLine::BuildDistanceLUT()
{
	const int32 N = RacingLinePoints.Num();
	CumulativeDistances.SetNum(N + 1);
	CumulativeDistances[0] = 0.0f;

	for (int32 I = 1; I <= N; ++I)
	{
		const FVector& A = RacingLinePoints[I - 1];
		const FVector& B = RacingLinePoints[I % N];
		CumulativeDistances[I] = CumulativeDistances[I - 1] + FVector::Dist(A, B);
	}
}

float UTurboRacingLine::DistanceToIndex(float Distance) const
{
	const int32 N = RacingLinePoints.Num();
	if (N == 0) return 0.0f;

	const float Total = CumulativeDistances[N];
	if (Total <= KINDA_SMALL_NUMBER) return 0.0f;

	// Wrap distance into [0, Total).
	Distance = FMath::Fmod(Distance, Total);
	if (Distance < 0.0f) Distance += Total;

	// Binary search for the segment.
	int32 Lo = 0;
	int32 Hi = N;
	while (Lo < Hi)
	{
		const int32 Mid = (Lo + Hi) / 2;
		if (CumulativeDistances[Mid + 1] < Distance)
		{
			Lo = Mid + 1;
		}
		else
		{
			Hi = Mid;
		}
	}

	const float SegStart = CumulativeDistances[Lo];
	const float SegEnd = CumulativeDistances[Lo + 1];
	const float SegLen = SegEnd - SegStart;
	const float Frac = (SegLen > KINDA_SMALL_NUMBER) ? (Distance - SegStart) / SegLen : 0.0f;

	return static_cast<float>(Lo) + Frac;
}

// -----------------------------------------------------------------------------
// Queries
// -----------------------------------------------------------------------------

FVector UTurboRacingLine::GetLocationAtDistance(float Distance) const
{
	return Optimizer.EvaluateCatmullRom(DistanceToIndex(Distance));
}

FVector UTurboRacingLine::GetDirectionAtDistance(float Distance) const
{
	// Finite-difference tangent via two close samples.
	const float Epsilon = 1.0f; // 1 cm — small enough for a smooth tangent.
	const FVector A = GetLocationAtDistance(Distance - Epsilon);
	const FVector B = GetLocationAtDistance(Distance + Epsilon);
	return (B - A).GetSafeNormal();
}

FVector UTurboRacingLine::GetLocationAtIndex(float Index) const
{
	return Optimizer.EvaluateCatmullRom(Index);
}

int32 UTurboRacingLine::FindClosestIndex(const FVector& WorldPosition) const
{
	return Optimizer.FindClosestIndex(WorldPosition);
}

float UTurboRacingLine::GetDistanceAtIndex(int32 Index) const
{
	if (!CumulativeDistances.IsValidIndex(Index))
	{
		return 0.0f;
	}
	return CumulativeDistances[Index];
}

float UTurboRacingLine::GetTotalDistance() const
{
	return CumulativeDistances.Num() > 0 ? CumulativeDistances.Last() : 0.0f;
}

// -----------------------------------------------------------------------------
// Debug Drawing
// -----------------------------------------------------------------------------

void UTurboRacingLine::DrawDebug() const
{
	if (!IsValid()) return;

	const UWorld* World = GetWorld();
	if (!World) return;

	const FVector Offset(0.0f, 0.0f, DebugLineZOffset);
	const int32 N = RacingLinePoints.Num();

	for (int32 I = 0; I < N; ++I)
	{
		const FVector& A = RacingLinePoints[I] + Offset;
		const FVector& B = RacingLinePoints[(I + 1) % N] + Offset;
		DrawDebugLine(World, A, B, DebugLineColor, false, -1.0f, 0, 50.0f);
	}
}

