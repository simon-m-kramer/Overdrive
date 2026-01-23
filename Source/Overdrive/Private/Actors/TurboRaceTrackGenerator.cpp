// Copyright Simon Kramer.All Rights Reserved.

#include "Actors/TurboRaceTrackGenerator.h"
#include "Components/SplineComponent.h"
#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"
#include "LandscapeSplineControlPoint.h"
#include "LandscapeSplineSegment.h"


ATurboRaceTrackGenerator::ATurboRaceTrackGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
}

void ATurboRaceTrackGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATurboRaceTrackGenerator::CopyFromLandscapeSpline()
{
    if (!SourceLandscapeSpline)
    {
        UE_LOG(LogTemp, Warning, TEXT("No SourceLandscapeSpline set"));
        return;
    }

    ALandscapeSplineActor* LSA = Cast<ALandscapeSplineActor>(SourceLandscapeSpline);
    if (!LSA)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor is not a LandscapeSplineActor"));
        return;
    }

    ULandscapeSplinesComponent* LandscapeSplines = LSA->GetSplinesComponent();
    if (!LandscapeSplines || LandscapeSplines->GetSegments().Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No segments found in LandscapeSplineActor"));
        return;
    }

    // Prepare for modification
    this->Modify();
    Spline->Modify();

    const TArray<TObjectPtr<ULandscapeSplineSegment>>& Segments = LandscapeSplines->GetSegments();
    TArray<FVector> Points;
    TSet<ULandscapeSplineSegment*> VisitedSegments;

    // Initial setup: Start at the first available segment
    ULandscapeSplineSegment* CurrentSegment = Segments[0];
    ULandscapeSplineControlPoint* CurrentPoint = CurrentSegment->Connections[0].ControlPoint;
    ULandscapeSplineControlPoint* StartPoint = CurrentPoint;

    do
    {
        // Add current point in World Space
        FVector WorldLocation = SourceLandscapeSpline->GetActorTransform().TransformPosition(CurrentPoint->Location);
        Points.Add(WorldLocation);

        ULandscapeSplineSegment* NextSegment = nullptr;
        ULandscapeSplineControlPoint* NextPoint = nullptr;

        // Walk through connected segments
        for (const FLandscapeSplineConnection& Connection : CurrentPoint->ConnectedSegments)
        {
            if (Connection.Segment && !VisitedSegments.Contains(Connection.Segment))
            {
                NextSegment = Connection.Segment;
                VisitedSegments.Add(NextSegment);

                // Determine which side of the segment we are moving toward
                NextPoint = (NextSegment->Connections[0].ControlPoint == CurrentPoint)
                    ? NextSegment->Connections[1].ControlPoint
                    : NextSegment->Connections[0].ControlPoint;
                break;
            }
        }

        CurrentPoint = NextPoint;

    } while (CurrentPoint && CurrentPoint != StartPoint);

    // Update the Spline Component
    if (Points.Num() > 0)
    {
        Spline->ClearSplinePoints();
        Spline->SetSplinePoints(Points, ESplineCoordinateSpace::World);

        // If the walk returned to the start, mark it as a closed loop
        bool bIsLoop = (CurrentPoint == StartPoint);
        Spline->SetClosedLoop(bIsLoop);

        Spline->UpdateSpline();
        UE_LOG(LogTemp, Log, TEXT("Successfully copied %d points. Loop: %s"), Points.Num(), bIsLoop ? TEXT("Yes") : TEXT("No"));
    }
}



