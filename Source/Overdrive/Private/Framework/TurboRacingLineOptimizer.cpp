// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboRacingLineOptimizer.h"

// -----------------------------------------------------------------------------
// Resample a polyline into Count evenly-spaced points (by arc length).
// Treats the input as a closed loop (wraps from last point back to first).
// -----------------------------------------------------------------------------
TArray<FVector> FTurboRacingLineOptimizer::ResamplePolyline(const TArray<FVector>& Points, int32 Count)
{
	const int32 N = Points.Num();
	if (N < 2 || Count < 2)
	{
		return Points;
	}

	// Build cumulative arc-length table (closed loop).
	TArray<float> CumulativeLen;
	CumulativeLen.SetNum(N + 1);
	CumulativeLen[0] = 0.0f;
	for (int32 I = 1; I <= N; ++I)
	{
		const FVector& A = Points[I - 1];
		const FVector& B = Points[I % N]; // wraps to first point
		CumulativeLen[I] = CumulativeLen[I - 1] + FVector::Dist(A, B);
	}

	const float TotalLen = CumulativeLen[N];
	const float Step = TotalLen / static_cast<float>(Count);

	TArray<FVector> Result;
	Result.Reserve(Count);

	int32 Seg = 0;
	for (int32 I = 0; I < Count; ++I)
	{
		const float TargetDist = I * Step;

		// Advance segment until we straddle TargetDist.
		while (Seg < N && CumulativeLen[Seg + 1] < TargetDist)
		{
			++Seg;
		}

		const float SegLen = CumulativeLen[Seg + 1] - CumulativeLen[Seg];
		const float Alpha = (SegLen > KINDA_SMALL_NUMBER)
			? (TargetDist - CumulativeLen[Seg]) / SegLen
			: 0.0f;

		const FVector& A = Points[Seg % N];
		const FVector& B = Points[(Seg + 1) % N];
		Result.Add(FMath::Lerp(A, B, Alpha));
	}

	return Result;
}

// -----------------------------------------------------------------------------
// Compute squared Menger curvature at sample I (with wrapping).
// kappa = 2 |cross(B-A, C-A)| / (|AB| |BC| |CA|)
// We return kappa^2 to avoid the sqrt and because we minimize sum(kappa^2).
// -----------------------------------------------------------------------------
float FTurboRacingLineOptimizer::ComputeSquaredCurvature(int32 I) const
{
	const int32 N = RacingLine.Num();
	const FVector& Prev = RacingLine[(I - 1 + N) % N];
	const FVector& Curr = RacingLine[I];
	const FVector& Next = RacingLine[(I + 1) % N];

	const FVector AB = Curr - Prev;
	const FVector AC = Next - Prev;
	const FVector BC = Next - Curr;

	const float CrossLen = FVector::CrossProduct(AB, AC).Size();
	const float Denom = AB.Size() * BC.Size() * AC.Size();

	if (Denom < KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float Kappa = 2.0f * CrossLen / Denom;
	return Kappa * Kappa;
}

// -----------------------------------------------------------------------------
// Rebuild world-space racing line from current alpha values.
// -----------------------------------------------------------------------------
void FTurboRacingLineOptimizer::RebuildLine()
{
	const int32 N = Alphas.Num();
	RacingLine.SetNum(N);
	for (int32 I = 0; I < N; ++I)
	{
		RacingLine[I] = FMath::Lerp(InnerBoundary[I], OuterBoundary[I], Alphas[I]);
	}
}

// -----------------------------------------------------------------------------
// Main optimization entry point.
// -----------------------------------------------------------------------------
void FTurboRacingLineOptimizer::Optimize(const TArray<FVector>& CenterlinePoints)
{
	if (CenterlinePoints.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("FTurboRacingLineOptimizer: Need at least 3 centerline points."));
		return;
	}

	// 1. Resample centerline to uniform spacing.
	const TArray<FVector> Centerline = ResamplePolyline(CenterlinePoints, NumSamples);
	const int32 N = Centerline.Num();

	// 2. Build inner/outer boundaries.
	//    For each sample, compute a lateral direction perpendicular to the
	//    local tangent.  We use the 2D (XY) perpendicular; the Z component
	//    is taken from the centerline so the line follows terrain.
	const float HalfWidth = TrackWidth * 0.5f;
	const float EffectiveHalf = HalfWidth - BoundaryMargin;

	InnerBoundary.SetNum(N);
	OuterBoundary.SetNum(N);

	for (int32 I = 0; I < N; ++I)
	{
		const FVector& Prev = Centerline[(I - 1 + N) % N];
		const FVector& Next = Centerline[(I + 1) % N];

		// Tangent along the track (forward direction).
		FVector Tangent = (Next - Prev).GetSafeNormal();

		// Lateral direction: perpendicular in the XY plane.
		// Convention: "right" is the outer boundary, "left" is inner.
		// (This is arbitrary — the optimizer doesn't care about the label,
		//  only that inner and outer are on opposite sides.)
		FVector Lateral = FVector(-Tangent.Y, Tangent.X, 0.0f).GetSafeNormal();

		// If the track has significant elevation change and you need the
		// lateral to stay in the road plane, replace the above with:
		//   FVector Up = FVector::UpVector;
		//   FVector Lateral = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

		InnerBoundary[I] = Centerline[I] - Lateral * EffectiveHalf;
		OuterBoundary[I] = Centerline[I] + Lateral * EffectiveHalf;
	}

	// 3. Initialize alphas to 0.5 (centerline).
	Alphas.SetNum(N);
	for (int32 I = 0; I < N; ++I)
	{
		Alphas[I] = 0.5f;
	}
	RebuildLine();

	// 4. Iterative relaxation.
	//    For each point, try nudging alpha in both directions and pick the
	//    value that minimizes local squared curvature (at I-1, I, I+1).
	const float StepSize = 0.01f;  // Alpha nudge per probe.
	const float MinAlpha = 0.0f;
	const float MaxAlpha = 1.0f;

	for (int32 Iter = 0; Iter < Iterations; ++Iter)
	{
		for (int32 I = 0; I < N; ++I)
		{
			const float OriginalAlpha = Alphas[I];

			// Cost = squared curvature at I only.
			// Each point optimizes its own curvature; neighbors handle theirs
			// on their own pass (Gauss-Seidel style).
			auto LocalCost = [&]() -> float
				{
					float Cost = ComputeSquaredCurvature(I);

					// Centerline regularization.
					if (CenterlineBias > 0.0f)
					{
						const float Dev = Alphas[I] - 0.5f;
						Cost += CenterlineBias * Dev * Dev;
					}
					return Cost;
				};

			// Evaluate current cost.
			const float CostCenter = LocalCost();

			// Probe left (toward inner boundary).
			const float AlphaLeft = FMath::Clamp(OriginalAlpha - StepSize, MinAlpha, MaxAlpha);
			Alphas[I] = AlphaLeft;
			RacingLine[I] = FMath::Lerp(InnerBoundary[I], OuterBoundary[I], AlphaLeft);
			const float CostLeft = LocalCost();

			// Probe right (toward outer boundary).
			const float AlphaRight = FMath::Clamp(OriginalAlpha + StepSize, MinAlpha, MaxAlpha);
			Alphas[I] = AlphaRight;
			RacingLine[I] = FMath::Lerp(InnerBoundary[I], OuterBoundary[I], AlphaRight);
			const float CostRight = LocalCost();

			// Pick the best.
			if (CostLeft < CostCenter && CostLeft <= CostRight)
			{
				Alphas[I] = AlphaLeft;
			}
			else if (CostRight < CostCenter)
			{
				Alphas[I] = AlphaRight;
			}
			else
			{
				Alphas[I] = OriginalAlpha;
			}

			RacingLine[I] = FMath::Lerp(InnerBoundary[I], OuterBoundary[I], Alphas[I]);
		}
	}
}

// -----------------------------------------------------------------------------
// Catmull-Rom evaluation for smooth interpolation between samples.
// -----------------------------------------------------------------------------
FVector FTurboRacingLineOptimizer::EvaluateCatmullRom(float T) const
{
	const int32 N = RacingLine.Num();
	if (N == 0) return FVector::ZeroVector;

	// Wrap T into [0, N).
	T = FMath::Fmod(T, static_cast<float>(N));
	if (T < 0.0f) T += static_cast<float>(N);

	const int32 I1 = static_cast<int32>(T) % N;
	const float F = T - FMath::FloorToFloat(T);

	const FVector& P0 = RacingLine[(I1 - 1 + N) % N];
	const FVector& P1 = RacingLine[I1];
	const FVector& P2 = RacingLine[(I1 + 1) % N];
	const FVector& P3 = RacingLine[(I1 + 2) % N];

	// Standard Catmull-Rom basis (tau = 0.5).
	const float F2 = F * F;
	const float F3 = F2 * F;

	return 0.5f * ((2.0f * P1)
		+ (-P0 + P2) * F
		+ (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * F2
		+ (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * F3);
}

// -----------------------------------------------------------------------------
// Brute-force closest-point lookup.
// -----------------------------------------------------------------------------
int32 FTurboRacingLineOptimizer::FindClosestIndex(const FVector& WorldPos) const
{
	int32 BestIdx = 0;
	float BestDistSq = MAX_flt;

	for (int32 I = 0; I < RacingLine.Num(); ++I)
	{
		const float DistSq = FVector::DistSquared(WorldPos, RacingLine[I]);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestIdx = I;
		}
	}

	return BestIdx;
}