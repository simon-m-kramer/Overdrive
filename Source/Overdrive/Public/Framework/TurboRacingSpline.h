// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboRacingLineCalculator.h"
#include "GameplayTagContainer.h"
#include "TurboRacingSpline.generated.h"

UCLASS()
class OVERDRIVE_API ATurboRacingSpline : public ATurboRacingLineCalculator
{
	GENERATED_BODY()

public:
	ATurboRacingSpline();

	USplineComponent* GetSplineComponent() const { return RacingLineSpline; }

	const FGameplayTagContainer& GetGameplayTags() const { return GameplayTags; }

	/** Converts spline distance into world location */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetLocationAtDistance(float Distance) const;

	/** Returns the normalized tangent at a given spline distance */
	UFUNCTION(BlueprintPure, Category = "Racing Line")
	FVector GetDirectionAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetSplineLength() const;

	UFUNCTION(BlueprintPure, Category = "Racing Line")
	bool IsClosedLoop() const { return bClosedLoop; }

	UFUNCTION(BlueprintPure, Category = "Racing Line")
	float GetTrackWidth() const { return HalfTrackWidth * 2.0f; }

	/** Computes the angle (in radians) between two tangents and divides by the distance between them (in cm). The return value is radians per cm. */
	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetCurvatureAtDistance(float Distance, float SampleRange = 400.0f) const;

	UFUNCTION(BlueprintPure, Category = "Curvature")
	float GetTurnSign(float Distance, float InLookaheadDistance = 200.0f) const;

protected:
	virtual void BeginPlay() override;

	/** Used to identify the spline */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer GameplayTags;

private:
	float WrapDistance(float Distance) const;


};