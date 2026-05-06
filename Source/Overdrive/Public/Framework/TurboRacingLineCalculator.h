// Copyright Simon Kramer. All Rights Reserved.

// The algorithm is based on:
// "Computer-Controlled Cars in Vamos" by Sam Varner
// https://vamos.sourceforge.net/computer-controlled-cars/node2.html

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurboRacingLineCalculator.generated.h"

class USplineComponent;

UCLASS()
class ATurboRacingLineCalculator : public AActor
{
	GENERATED_BODY()

public:
	ATurboRacingLineCalculator();

	// --- Splines ---------------------------------------------------------

	/** The track centerline. Edit this spline to define track shape. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing Line")
	TObjectPtr<USplineComponent> CenterlineSpline;

	/** The computed racing line. Populated when calculate is clicked */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing Line")
	TObjectPtr<USplineComponent> RacingLineSpline;

	// --- Track Parameters ------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "Racing Line|Track", meta = (ClampMin = "50.0", Units = "cm"))
	float NodeSpacing = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Track", meta = (ClampMin = "50.0", Units = "cm"))
	float HalfTrackWidth = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Track")
	bool bClosedLoop = true;

	// --- Simulation Parameters ----------------------------------------------

	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float Stiffness = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float Damping = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "1"))
	int32 Iterations = 1000;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float DeltaTime = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.001"))
	float Mass = 1.0f;

	// --- Actions ------------------------------------------------------------

	UFUNCTION(CallInEditor, Category = "Racing Line")
	void CalculateRacingLine();

	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FVector> GetRacingLinePositions() const { return CachedPositions; }

private:
	struct FSimNode
	{
		FVector CenterPos;
		FVector Perpendicular;
		float Offset;
		float Velocity;

		FVector GetPosition() const
		{
			return CenterPos + Offset * Perpendicular;
		}
	};

	/** Cached results from last calculation. However not used right now, AI queries spline directly */
	TArray<FVector> CachedPositions;

	/** Conversion factors */
	static constexpr float CmToM = 0.01f;
	static constexpr float MToCm = 100.0f;
};