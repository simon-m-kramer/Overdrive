// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "TurboRacingSpline.generated.h"

class USplineComponent;

UCLASS()
class OVERDRIVE_API ATurboRacingSpline : public AActor
{
	GENERATED_BODY()

public:
	ATurboRacingSpline();

	USplineComponent* GetSplineComponent() const { return Spline; }
	const FGameplayTagContainer& GetGameplayTags() const { return GameplayTags; }

	// =========================================================================
	// SPLINE QUERIES
	// =========================================================================

	/** Get world-space location on the racing line at a given distance */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetLocationAtDistance(float Distance) const;

	/** Get world-space direction (tangent) at a given distance */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetDirectionAtDistance(float Distance) const;

	/** Get the total spline length */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetSplineLength() const;

	/** Get whether the spline is a closed loop */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	bool IsClosedLoop() const;

	// =========================================================================
	// CURVATURE ANALYSIS
	// =========================================================================

	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetCurvatureAtDistance(float Distance, float SampleRange = 300.0f) const;

	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetTurnSign(float Distance, float InLookaheadDistance = 200.0f) const;

	float GetMaxTrackCurvature() const { return MaxTrackCurvature; }
	float GetMinCurvatureThreshold() const { return MinCurvatureThreshold; }

	// =========================================================================
	// TRACK CONFIGURATION
	// =========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
	float TrackWidth = 1200.0f;

	// =========================================================================
	// CURVATURE CONFIGURATION
	// =========================================================================

	UPROPERTY(EditAnywhere, Category = "Curvature")
	float CurvatureSampleRange = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Curvature", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float MinCurvatureThreshold = 0.05f;

	// =========================================================================
	// DEBUG
	// =========================================================================

	void DrawDebugRacingLine(UWorld* World) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer GameplayTags;

private:
	float WrapDistance(float Distance) const;
	void CalculateMaxCurvature();

	float MaxTrackCurvature = 0.0f;
};