// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Evade.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"
#include "Components/TurboVehicleDetectionComponent.h"

UTurboAction_Evade::UTurboAction_Evade()
{
    ActionName = TEXT("Evade");
    ActionTag = TurboGameplayTags::Action_Evade;
    BlocksTags.AddTag(TurboGameplayTags::Action_Evade);
    BlocksTags.AddTag(TurboGameplayTags::Action_Yield);
    BlocksTags.AddTag(TurboGameplayTags::Action_Overtake);
}

bool UTurboAction_Evade::CanActivate(const FTurboDecisionContext& Context) const
{
    // Activate if there's a car beside us
    if (Context.bLeftClear && Context.bRightClear)
    {
        return false;
    }

    // Check if we have room to evade
    if (!Context.bLeftClear)
    {
        // Car on left, would evade right
        float ProjectedPosition = Context.SignedDistanceFromCenter + LateralOffset;
        if (FMath::Abs(ProjectedPosition) > Context.TrackHalfWidth * 0.9f)
        {
            return false;
        }
    }
    else
    {
        // Car on right, would evade left
        float ProjectedPosition = Context.SignedDistanceFromCenter - LateralOffset;
        if (FMath::Abs(ProjectedPosition) > Context.TrackHalfWidth * 0.9f)
        {
            return false;
        }
    }

    return true;
}

void UTurboAction_Evade::Start(bool bFirstTime)
{
    Super::Start(bFirstTime);

    if (bFirstTime)
    {
        CurrentLateralOffset = 0.0f;
        TimeInEvade = 0.0f;
        TimeSinceClear = 0.0f;
        bEvadeComplete = false;

        // Determine evade direction based on where car is
        bEvadingLeft = IsCarOnRight();

        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(58, 3.0f, FColor::Orange,
                FString::Printf(TEXT("EVADE STARTED - Direction: %s"),
                    bEvadingLeft ? TEXT("LEFT") : TEXT("RIGHT")));
        }
    }
}

void UTurboAction_Evade::Update(float DeltaTime)
{
    TimeInEvade += DeltaTime;

    if (IsCarBeside())
    {
        TimeSinceClear = 0.0f;

        // Blend toward target offset
        float TargetOffset = bEvadingLeft ? -LateralOffset : LateralOffset;
        CurrentLateralOffset = FMath::FInterpTo(CurrentLateralOffset, TargetOffset, DeltaTime, OffsetBlendSpeed);
    }
    else
    {
        TimeSinceClear += DeltaTime;

        // Blend back to center
        CurrentLateralOffset = FMath::FInterpTo(CurrentLateralOffset, 0.0f, DeltaTime, OffsetBlendSpeed);
    }

    Super::Update(DeltaTime);

    if (bDrawDebug)
    {
        GEngine->AddOnScreenDebugMessage(59, 0.0f, FColor::Orange,
            FString::Printf(TEXT("Evade: Offset=%.0f | Clear=%.1fs"),
                CurrentLateralOffset, TimeSinceClear));
    }
}

bool UTurboAction_Evade::IsDone()
{
    if (bEvadeComplete)
    {
        return true;
    }

    // Timeout
    if (TimeInEvade > EvadeTimeout)
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(59, 3.0f, FColor::Red, TEXT("EVADE TIMEOUT"));
        }
        bEvadeComplete = true;
        return true;
    }

    // Car has been clear and we've returned to center
    if (TimeSinceClear > ClearDuration && FMath::Abs(CurrentLateralOffset) < 10.0f)
    {
        if (bDrawDebug)
        {
            GEngine->AddOnScreenDebugMessage(59, 3.0f, FColor::Green, TEXT("EVADE COMPLETE"));
        }
        bEvadeComplete = true;
        return true;
    }

    return false;
}

FVector UTurboAction_Evade::GetTargetPoint()
{
    FVector BasePoint = Super::GetTargetPoint();

    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return BasePoint;
    }

    if (FMath::Abs(CurrentLateralOffset) < 1.0f)
    {
        return BasePoint;
    }

    USplineComponent* Spline = RacingSplineActor->GetSplineComponent();
    if (!Spline)
    {
        return BasePoint;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float LookaheadDist = GetLookaheadDistance();
    float TargetDistance = CurrentDistance + LookaheadDist;

    float SplineLength = Spline->GetSplineLength();
    if (Spline->IsClosedLoop() && TargetDistance >= SplineLength)
    {
        TargetDistance = FMath::Fmod(TargetDistance, SplineLength);
    }

    FVector Tangent = Spline->GetDirectionAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Up = Spline->GetUpVectorAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
    FVector Right = FVector::CrossProduct(Tangent, Up).GetSafeNormal();

    return BasePoint + (Right * CurrentLateralOffset);
}

bool UTurboAction_Evade::IsCarBeside() const
{
    return IsCarOnLeft() || IsCarOnRight();
}

bool UTurboAction_Evade::IsCarOnLeft() const
{
    if (!Vehicle.IsValid())
    {
        return false;
    }

    UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
    return Detection && Detection->IsCarOnLeft();
}

bool UTurboAction_Evade::IsCarOnRight() const
{
    if (!Vehicle.IsValid())
    {
        return false;
    }

    UTurboVehicleDetectionComponent* Detection = Vehicle->GetDetectionComponent();
    return Detection && Detection->IsCarOnRight();
}

