// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/TurboRacingLineOptimizer.h"
#include "TurboRacingLine.generated.h"

class USplineComponent;

/**
 * Component that generates and provides access to an optimized racing line.
 *
 * Attach this to any actor that has (or can reference) a USplineComponent
 * representing the track centerline.  Call GenerateRacingLine() to run the
 * optimizer — either at edit-time via the button in Details, at BeginPlay,
 * or on demand.
 *
 * Typical setup:
 *   1. Place this component on your track actor.
 *   2. Point CenterlineSpline at the track's spline component.
 *   3. Set TrackWidth and tweak optimizer settings.
 *   4. Call GenerateRacingLine() (or enable bGenerateOnBeginPlay).
 *   5. Query the line with GetLocationAtDistance() or GetLocationAtIndex().
 */
UCLASS(ClassGroup = (Turbo), meta = (BlueprintSpawnableComponent))
class UTurboRacingLine : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurboRacingLine();

	// -- Configuration ----------------------------------------------------------

	/** The spline component that defines the track centerline. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Racing Line")
	TObjectPtr<USplineComponent> CenterlineSpline;

	/** Full track width in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float TrackWidth = 1200.0f;

	/** Optimizer settings (sample count, iterations, bias, margin). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	FTurboRacingLineOptimizer Optimizer;

	/** If true, the racing line is generated automatically in BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	bool bGenerateOnBeginPlay = true;

	/** If true, draws the racing line as a debug polyline at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line|Debug")
	bool bDrawDebugLine = true;

	/** Color of the debug racing line. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line|Debug", meta = (EditCondition = "bDrawDebugLine"))
	FColor DebugLineColor = FColor::Green;

	/** Height offset for the debug line so it doesn't z-fight with the road. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line|Debug", meta = (EditCondition = "bDrawDebugLine"))
	float DebugLineZOffset = 10.0f;

	// -- Core Interface ----------------------------------------------------------

	/**
	 * Sample the centerline spline and run the optimizer.
	 * Can be called at any time — results are available immediately after.
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	void GenerateRacingLine();

	/** True if GenerateRacingLine() has been called and produced a valid result. */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	bool IsValid() const { return RacingLinePoints.Num() > 2; }

	// -- Queries ----------------------------------------------------------

	/**
	 * Get a world-space location on the racing line by distance along it.
	 * Uses Catmull-Rom interpolation for smooth results.
	 *
	 * @param Distance  Distance in cm from the start of the line.
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetLocationAtDistance(float Distance) const;

	/**
	 * Get the forward direction (tangent) of the racing line at a given distance.
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetDirectionAtDistance(float Distance) const;

	/**
	 * Get a world-space location by fractional index (Catmull-Rom).
	 * @param Index  Value in [0, NumSamples). Wraps automatically.
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetLocationAtIndex(float Index) const;

	/**
	 * Find the index of the closest racing line point to a world position.
	 * Useful as a starting point for more refined distance-along-track queries.
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	int32 FindClosestIndex(const FVector& WorldPosition) const;

	/**
	 * Approximate distance along the racing line for a given point index.
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetDistanceAtIndex(int32 Index) const;

	/** Total arc length of the racing line in cm. */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetTotalDistance() const;

	/** Direct read access to the raw point array. */
	const TArray<FVector>& GetRacingLinePoints() const { return RacingLinePoints; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Sample the centerline spline into an array of world-space points. */
	TArray<FVector> SampleCenterline() const;

	/** Build the cumulative distance LUT from the current racing line points. */
	void BuildDistanceLUT();

	/** Convert a distance to a fractional index using the LUT. */
	float DistanceToIndex(float Distance) const;

	/** Draw the debug racing line. */
	void DrawDebug() const;

	// -- Cached Data ----------------------------------------------------------

	/** The optimized racing line points (world-space). */
	TArray<FVector> RacingLinePoints;

	/**
	 * Cumulative arc-length at each point index.
	 * CumulativeDistances[0] = 0, CumulativeDistances[N] = total lap length.
	 * Has N+1 entries (last entry wraps back to first point).
	 */
	TArray<float> CumulativeDistances;
};