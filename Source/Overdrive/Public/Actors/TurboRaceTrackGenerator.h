// Copyright Simon Kramer.All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboRacingSpline.h"
#include "TurboRaceTrackGenerator.generated.h"



UCLASS()
class OVERDRIVE_API ATurboRaceTrackGenerator : public ATurboRacingSpline
{
	GENERATED_BODY()
	
public:	
	ATurboRaceTrackGenerator();

protected:
	virtual void BeginPlay() override;

    // Use AActor here to make it easier to pick in the viewport
    UPROPERTY(EditAnywhere, Category = "Setup")
    AActor* SourceLandscapeSpline;

    UFUNCTION(CallInEditor, Category = "Setup")
    void CopyFromLandscapeSpline();

};
