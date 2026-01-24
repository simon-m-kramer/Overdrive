// Copyright Simon Kramer.All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
#include "Framework/TurboRacingSpline.h"
#include "TurboRaceTrackGenerator.generated.h"

//class USplineComponent;


UCLASS()
class OVERDRIVE_API ATurboRaceTrackGenerator : public ATurboRacingSpline
{
	GENERATED_BODY()
	
public:	
	ATurboRaceTrackGenerator();

protected:
	virtual void BeginPlay() override;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    //USplineComponent* Spline;

    // Use AActor here to make it easier to pick in the viewport
    UPROPERTY(EditAnywhere, Category = "Setup")
    AActor* SourceLandscapeSpline;

    UFUNCTION(CallInEditor, Category = "Setup")
    void CopyFromLandscapeSpline();

};
