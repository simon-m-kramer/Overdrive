// Copyright Simon Kramer.All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboRacingSpline.h"
#include "TurboRaceTrackGenerator.generated.h"

class UTurboRacingLine;

UCLASS()
class OVERDRIVE_API ATurboRaceTrackGenerator : public ATurboRacingSpline
{
	GENERATED_BODY()
	
public:	
	ATurboRaceTrackGenerator();

protected:
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Setup")
    AActor* SourceLandscapeSpline;

    UFUNCTION(CallInEditor, Category = "Setup")
    void CopyFromLandscapeSpline();

	UPROPERTY(EditAnywhere, Category = "Setup")
	TObjectPtr<UTurboRacingLine> RacingLine;

};
