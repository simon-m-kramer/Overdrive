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

	/** The track centerline. Edit this spline to define your track shape. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing Line")
	TObjectPtr<USplineComponent> CenterlineSpline;

	/** The computed racing line. Populated when you click Calculate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing Line")
	TObjectPtr<USplineComponent> RacingLineSpline;

	// --- Track Parameters ------------------------------------------------

	/** Distance between simulation nodes along the centerline in cm. */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Track", meta = (ClampMin = "50.0", Units = "cm"))
	float NodeSpacing = 800.0f;

	/** Half the track width in cm. Nodes cannot move further than this from the centerline in either direction. */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Track", meta = (ClampMin = "50.0", Units = "cm"))
	float HalfTrackWidth = 600.0f;

	/** Whether the track is a closed loop. */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Track")
	bool bClosedLoop = true;

	// --- Simulation Parameters ----------------------------------------------
	// These defaults match the reference document

	/** Spring stiffness of the hinges */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float Stiffness = 1.0f;

	/** Velocity damping coefficient */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float Damping = 0.1f;

	/** Number of simulation iterations */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "1"))
	int32 Iterations = 1000;

	/** Simulation time step */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.01"))
	float DeltaTime = 1.0f;

	/** Node mass */
	UPROPERTY(EditAnywhere, Category = "Racing Line|Simulation", meta = (ClampMin = "0.001"))
	float Mass = 1.0f;

	// --- Actions ------------------------------------------------------------

	/** Run the racing line calculation. */
	UFUNCTION(CallInEditor, Category = "Racing Line")
	void CalculateRacingLine();

	/** Get the racing line positions (in world space, cm). Call CalculateRacingLine() first. */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FVector> GetRacingLinePositions() const;

private:
	/** Internal node representation for the simulation. */
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