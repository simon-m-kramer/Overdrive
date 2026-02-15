// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TurboRacingLineOptimizer.generated.h"

/**
 * Geometric racing line optimizer.
 *
 * Given a sampled centerline and a track width, computes a minimum-curvature
 * racing line using iterative relaxation.  The result is a set of world-space
 * points that can be fed into a spline, interpolated with Catmull-Rom, or
 * used directly as a point array.
 *
 * Usage:
 *   FTurboRacingLineOptimizer Optimizer;
 *   Optimizer.TrackWidth      = 1200.0f;
 *   Optimizer.NumSamples      = 400;
 *   Optimizer.Iterations      = 500;
 *   Optimizer.Optimize(CenterlineSplinePoints);
 *   const TArray<FVector>& Line = Optimizer.GetRacingLine();
 */
USTRUCT(BlueprintType)
struct FTurboRacingLineOptimizer
{
	GENERATED_BODY()

	// -- Parameters -------------------------------------------------------------

	/** Full track width in cm. Each side extends TrackWidth/2 from center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float TrackWidth = 1200.0f;

	/** Number of evenly-spaced sample points along the centerline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line", meta = (ClampMin = "20", ClampMax = "5000"))
	int32 NumSamples = 400;

	/** Number of relaxation iterations. More = smoother, diminishing returns past ~500. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line", meta = (ClampMin = "1"))
	int32 Iterations = 500;

	/**
	 * Centerline regularization weight in [0, 1].
	 * 0  = pure curvature minimization (aggressive corner cutting).
	 * >0 = penalizes deviation from the centerline to prevent the line from
	 *      hugging inner walls on long sweeping curves.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CenterlineBias = 0.02f;

	/**
	 * Track boundary margin in cm.  The optimizer keeps the racing line at
	 * least this far from the inner/outer edge.  Useful to avoid placing the
	 * line right on a kerb or wall.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line", meta = (ClampMin = "0.0"))
	float BoundaryMargin = 50.0f;

	// -- Interface -------------------------------------------------------------

	/**
	 * Run the optimizer.
	 *
	 * @param CenterlinePoints  Ordered world-space points that describe the
	 *                          track centerline (must form a closed loop —
	 *                          first and last point should be near-identical
	 *                          or the array is treated as wrapping).
	 */
	void Optimize(const TArray<FVector>& CenterlinePoints);

	/** The optimized racing line (world-space, same sample count as NumSamples). */
	const TArray<FVector>& GetRacingLine() const { return RacingLine; }

	/** The alpha values in [0,1] per sample (0 = inner edge, 1 = outer edge). */
	const TArray<float>& GetAlphas() const { return Alphas; }

	// -- Utility -------------------------------------------------------------

	/**
	 * Evaluate the racing line at an arbitrary fractional index using
	 * Catmull-Rom interpolation (C1 continuous).
	 *
	 * @param T  A value in [0, NumSamples).  Integer part selects the segment,
	 *           fractional part interpolates within it.
	 */
	FVector EvaluateCatmullRom(float T) const;

	/**
	 * Given a world position, find the closest point index on the racing line
	 * (brute-force, suitable for infrequent queries or small sample counts).
	 */
	int32 FindClosestIndex(const FVector& WorldPos) const;

private:
	/** Resample a polyline to produce Count evenly-spaced points. */
	static TArray<FVector> ResamplePolyline(const TArray<FVector>& Points, int32 Count);

	/** Compute the discrete squared curvature at index I (wrapping). */
	float ComputeSquaredCurvature(int32 I) const;

	/** Rebuild the world-space racing line from current alphas. */
	void RebuildLine();

	// -- Data -------------------------------------------------------------

	TArray<FVector> InnerBoundary;
	TArray<FVector> OuterBoundary;
	TArray<float>   Alphas;
	TArray<FVector> RacingLine;
};