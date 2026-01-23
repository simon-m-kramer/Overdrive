// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIVehicle.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"


ATurboAIVehicle::ATurboAIVehicle()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ATurboAIVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

    FindTrackSpline();
}

void ATurboAIVehicle::FindTrackSpline()
{
	if (TrackSpline)
	{
		return;
	}

    // Development fallback
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (USplineComponent* Spline = Actor->FindComponentByClass<USplineComponent>())
        {
            TrackSpline = Spline;
            UE_LOG(LogTemp, Warning, TEXT("TurboAIVehicle: Using fallback spline on %s"), *Actor->GetName());
            return;
        }
    }
}
