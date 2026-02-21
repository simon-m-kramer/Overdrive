// Copyright Simon Kramer. All Rights Reserved.

#include "Framework/TurboRaceManager.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
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

    // TO DO: Change that to find by tag
    // Find the racing spline
    for (TActorIterator<ATurboRacingSpline> It(GetWorld()); It; ++It)
    {
        RacingSpline = *It;
        break;
    }

    if (RacingSpline && RacingSpline->GetSplineComponent())
    {
        SplineLength = RacingSpline->GetSplineComponent()->GetSplineLength();
    }

    CollectVehicles();
}

void UTurboRaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateProgress();
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
        Entry.CurrentLap = 0;  // simple fix to have it count properly
        Entry.Placement = Entries.Num() + 1;

        // Initialize spline distance
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

void UTurboRaceManager::UpdateProgress()
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

        DetectLapCompletion(Entry, SplineLength);

        Entry.TotalProgress = (Entry.CurrentLap - 1) * SplineLength + Entry.SplineDistance;
    }
}

void UTurboRaceManager::DetectLapCompletion(FRaceEntry& Entry, float InSplineLength)
{
    // Detect crossing the start/finish line
    // Large backward jump in spline distance means we wrapped around
    float Delta = Entry.SplineDistance - Entry.PreviousSplineDistance;

    // Crossed finish line going forward (large negative delta = wrapped from end to start)
    if (Delta < -InSplineLength * 0.5f)
    {
        Entry.CurrentLap++;

        if (bDrawDebug)
        {
            UE_LOG(LogTemp, Warning, TEXT("RaceManager: %s completed lap %d"),
                *Entry.Vehicle->GetName(), Entry.CurrentLap - 1);
        }
    }

    // Crossed finish line going backward (large positive delta = wrapped from start to end)
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

