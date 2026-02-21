// Copyright Simon Kramer. All Rights Reserved.

#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UTurboRaceManager::UTurboRaceManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurboRaceManager::BeginPlay()
{
	Super::BeginPlay();

	FindRacingSpline();

	if (RacingSpline && RacingSpline->GetSplineComponent())
	{
		SplineLength = RacingSpline->GetSplineComponent()->GetSplineLength();
	}

	CollectVehicles();
}

void UTurboRaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateProgress(DeltaTime);
	SortStandings();

	if (bDrawDebug)
	{
		for (const FRaceEntry& Entry : Entries)
		{
			if (Entry.Vehicle.IsValid())
			{
				FString Name = Entry.Vehicle->VehicleName;
				GEngine->AddOnScreenDebugMessage(100 + Entry.Placement, 0.0f, FColor::White,
					FString::Printf(TEXT("P%d: %s | Lap %d/%d | Progress: %.0f"),
						Entry.Placement, *Name, Entry.CurrentLap, TotalLaps, Entry.TotalProgress));
			}
		}
	}
}

// =============================================================================
// SPLINE
// =============================================================================

void UTurboRaceManager::FindRacingSpline()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATurboRacingSpline::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		ATurboRacingSpline* SplineActor = Cast<ATurboRacingSpline>(Actor);
		if (SplineActor && SplineActor->GetGameplayTags().HasTag(TurboGameplayTags::Track_MainSpline))
		{
			RacingSpline = SplineActor;
			break;
		}
	}
}

// =============================================================================
// VEHICLES
// =============================================================================

void UTurboRaceManager::CollectVehicles()
{
	Entries.Empty();

	for (TActorIterator<ATurboVehicle> It(GetWorld()); It; ++It)
	{
		ATurboVehicle* Vehicle = *It;
		if (!Vehicle)
		{
			continue;
		}

		FRaceEntry Entry;
		Entry.Vehicle = Vehicle;
		Entry.CurrentLap = 0;
		Entry.Placement = Entries.Num() + 1;

		if (RacingSpline && RacingSpline->GetSplineComponent())
		{
			USplineComponent* Spline = RacingSpline->GetSplineComponent();
			FVector Location = Vehicle->GetActorLocation();
			float Distance = Spline->GetDistanceAlongSplineAtLocation(Location, ESplineCoordinateSpace::World);
			Entry.SplineDistance = Distance;
			Entry.PreviousSplineDistance = Distance;
		}

		Entries.Add(Entry);
	}

	UE_LOG(LogTemp, Log, TEXT("RaceManager: Collected %d vehicles"), Entries.Num());
}

// =============================================================================
// PROGRESS & LAP TIMING
// =============================================================================

void UTurboRaceManager::UpdateProgress(float DeltaTime)
{
	if (!RacingSpline || !RacingSpline->GetSplineComponent())
	{
		return;
	}

	USplineComponent* Spline = RacingSpline->GetSplineComponent();

	for (FRaceEntry& Entry : Entries)
	{
		if (!Entry.Vehicle.IsValid())
		{
			continue;
		}

		Entry.PreviousSplineDistance = Entry.SplineDistance;

		FVector Location = Entry.Vehicle->GetActorLocation();
		Entry.SplineDistance = Spline->GetDistanceAlongSplineAtLocation(Location, ESplineCoordinateSpace::World);

		if (Entry.bLapTimingStarted)
		{
			Entry.CurrentLapTime += DeltaTime;
		}

		DetectLapCompletion(Entry, SplineLength);

		Entry.TotalProgress = (Entry.CurrentLap - 1) * SplineLength + Entry.SplineDistance;
	}
}

void UTurboRaceManager::DetectLapCompletion(FRaceEntry& Entry, float InSplineLength)
{
	float Delta = Entry.SplineDistance - Entry.PreviousSplineDistance;

	if (Delta < -InSplineLength * 0.5f)
	{
		if (Entry.bLapTimingStarted)
		{
			Entry.LastLapTime = Entry.CurrentLapTime;

			if (Entry.BestLapTime <= 0.0f || Entry.CurrentLapTime < Entry.BestLapTime)
			{
				Entry.BestLapTime = Entry.CurrentLapTime;
			}
		}

		Entry.CurrentLapTime = 0.0f;
		Entry.bLapTimingStarted = true;
		Entry.CurrentLap++;

		if (bDrawDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("RaceManager: %s completed lap %d"),
				*Entry.Vehicle->GetName(), Entry.CurrentLap - 1);
		}
	}

	if (Delta > InSplineLength * 0.5f)
	{
		Entry.CurrentLap = FMath::Max(1, Entry.CurrentLap - 1);

		if (bDrawDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("RaceManager: %s went back a lap to %d"),
				*Entry.Vehicle->GetName(), Entry.CurrentLap);
		}
	}
}

// =============================================================================
// STANDINGS
// =============================================================================

void UTurboRaceManager::SortStandings()
{
	Entries.Sort([](const FRaceEntry& A, const FRaceEntry& B)
		{
			return A.TotalProgress > B.TotalProgress;
		});

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Entries[i].Placement = i + 1;
	}
}

// =============================================================================
// QUERIES
// =============================================================================

int32 UTurboRaceManager::GetPlacement(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.Placement;
		}
	}
	return 0;
}

int32 UTurboRaceManager::GetCurrentLap(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.CurrentLap;
		}
	}
	return 0;
}

bool UTurboRaceManager::HasFinished(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.CurrentLap > TotalLaps;
		}
	}
	return false;
}

float UTurboRaceManager::GetCurrentLapTime(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.CurrentLapTime;
		}
	}
	return 0.0f;
}

float UTurboRaceManager::GetLastLapTime(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.LastLapTime;
		}
	}
	return 0.0f;
}

float UTurboRaceManager::GetBestLapTime(ATurboVehicle* Vehicle) const
{
	for (const FRaceEntry& Entry : Entries)
	{
		if (Entry.Vehicle == Vehicle)
		{
			return Entry.BestLapTime;
		}
	}
	return 0.0f;
}

FString UTurboRaceManager::FormatLapTime(float TimeSeconds)
{
	const int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	const float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
	return FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds);
}

