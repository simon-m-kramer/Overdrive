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

	// Getter for AI Controllers to easily grab the spline
	USplineComponent* GetSplineComponent() const { return Spline; }

	// Returns the tags associated with this track segment
	const FGameplayTagContainer& GetGameplayTags() const { return GameplayTags; }

	// =====================================================================
	// CURVATURE ANALYSIS
	// =====================================================================

	UFUNCTION()
	float GetCurvatureAtDistance(float Distance, float SampleRange = 100.0f) const;

	UFUNCTION()
	float GetTurnSign(float Distance) const;


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer GameplayTags;

};
