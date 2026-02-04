// Copyright Simon Kramer. All Rights Reserved.


#include "Components/TurboVehicleDetectionComponent.h"
#include "Framework/TurboVehicle.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"


UTurboVehicleDetectionComponent::UTurboVehicleDetectionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTurboVehicleDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerVehicle = Cast<ATurboVehicle>(GetOwner());
}

void UTurboVehicleDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerVehicle)
    {
        return;
    }

    UpdateForwardDetection();
    UpdateBehindDetection();
    UpdateSideDetection();
}

void UTurboVehicleDetectionComponent::UpdateForwardDetection()
{
    bCarAhead = false;
    DistanceToCarAhead = 0.0f;
    CarAhead = nullptr;

    FVector Start = OwnerVehicle->GetActorLocation();
    FVector Forward = OwnerVehicle->GetActorForwardVector();
    FVector End = Start + (Forward * ForwardDetectionRange);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerVehicle);

    FHitResult HitResult;
    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        DetectionChannel,
        FCollisionShape::MakeSphere(ForwardTraceRadius),
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        ATurboVehicle* HitVehicle = Cast<ATurboVehicle>(HitResult.GetActor());
        if (HitVehicle)
        {
            bCarAhead = true;
            DistanceToCarAhead = HitResult.Distance;
            CarAhead = HitVehicle;
        }
    }

    if (bDrawDebug)
    {
        FColor DebugColor = bCarAhead ? FColor::Red : FColor::Green;
        DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.0f, 0, 2.0f);

        if (bCarAhead)
        {
            DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 50.0f, 8, FColor::Red, false, 0.0f);

            GEngine->AddOnScreenDebugMessage(20, 0.0f, FColor::Red,
                FString::Printf(TEXT("Car ahead: %.0f cm"), DistanceToCarAhead));
        }
    }
}

void UTurboVehicleDetectionComponent::UpdateBehindDetection()
{
    bCarBehind = false;
    DistanceToCarBehind = 0.0f;
    CarBehind = nullptr;

    FVector Start = OwnerVehicle->GetActorLocation();
    FVector Backward = -OwnerVehicle->GetActorForwardVector();
    FVector End = Start + (Backward * BehindDetectionRange);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerVehicle);

    FHitResult HitResult;
    bool bHit = GetWorld()->SweepSingleByChannel(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        DetectionChannel,
        FCollisionShape::MakeSphere(BehindTraceRadius),
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        ATurboVehicle* HitVehicle = Cast<ATurboVehicle>(HitResult.GetActor());
        if (HitVehicle)
        {
            bCarBehind = true;
            DistanceToCarBehind = HitResult.Distance;
            CarBehind = HitVehicle;
        }
    }

    if (bDrawDebug)
    {
        FColor DebugColor = bCarBehind ? FColor::Orange : FColor::Green;
        DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.0f, 0, 2.0f);

        if (bCarBehind)
        {
            DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 50.0f, 8, FColor::Orange, false, 0.0f);

            GEngine->AddOnScreenDebugMessage(21, 0.0f, FColor::Orange,
                FString::Printf(TEXT("Car behind: %.0f cm"), DistanceToCarBehind));
        }
    }
}

void UTurboVehicleDetectionComponent::UpdateSideDetection()
{
    bCarOnLeft = false;
    bCarOnRight = false;

    FVector VehicleLocation = OwnerVehicle->GetActorLocation();
    FVector Forward = OwnerVehicle->GetActorForwardVector();
    FVector Right = OwnerVehicle->GetActorRightVector();
    FQuat VehicleRotation = OwnerVehicle->GetActorQuat();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerVehicle);

    FVector BoxExtent(SideDetectionLength * 0.5f, SideDetectionWidth * 0.5f, SideDetectionHeight * 0.5f);
    FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

    // Left side detection
    FVector LeftCenter = VehicleLocation + (Right * -SideDetectionOffset);

    TArray<FOverlapResult> LeftOverlaps;
    bool bLeftHit = GetWorld()->OverlapMultiByChannel(
        LeftOverlaps,
        LeftCenter,
        VehicleRotation,
        DetectionChannel,
        BoxShape,
        QueryParams
    );

    if (bLeftHit)
    {
        for (const FOverlapResult& Overlap : LeftOverlaps)
        {
            ATurboVehicle* HitVehicle = Cast<ATurboVehicle>(Overlap.GetActor());
            if (HitVehicle && HitVehicle != OwnerVehicle)
            {
                bCarOnLeft = true;
                break;
            }
        }
    }

    // Right side detection
    FVector RightCenter = VehicleLocation + (Right * SideDetectionOffset);

    TArray<FOverlapResult> RightOverlaps;
    bool bRightHit = GetWorld()->OverlapMultiByChannel(
        RightOverlaps,
        RightCenter,
        VehicleRotation,
        DetectionChannel,
        BoxShape,
        QueryParams
    );

    if (bRightHit)
    {
        for (const FOverlapResult& Overlap : RightOverlaps)
        {
            ATurboVehicle* HitVehicle = Cast<ATurboVehicle>(Overlap.GetActor());
            if (HitVehicle && HitVehicle != OwnerVehicle)
            {
                bCarOnRight = true;
                break;
            }
        }
    }

    if (bDrawDebug)
    {
        FColor LeftColor = bCarOnLeft ? FColor::Red : FColor::Green;
        FColor RightColor = bCarOnRight ? FColor::Red : FColor::Green;

        DrawDebugBox(GetWorld(), LeftCenter, BoxExtent, VehicleRotation, LeftColor, false, 0.0f, 0, 2.0f);
        DrawDebugBox(GetWorld(), RightCenter, BoxExtent, VehicleRotation, RightColor, false, 0.0f, 0, 2.0f);

        if (bCarOnLeft)
        {
            GEngine->AddOnScreenDebugMessage(22, 0.0f, FColor::Red, TEXT("Car on LEFT"));
        }
        if (bCarOnRight)
        {
            GEngine->AddOnScreenDebugMessage(23, 0.0f, FColor::Red, TEXT("Car on RIGHT"));
        }
    }
}

bool UTurboVehicleDetectionComponent::IsOvertakeSafe(EOvertakeSide Side) const
{
    if (Side == EOvertakeSide::Left)
    {
        return !bCarOnLeft;
    }
    else
    {
        return !bCarOnRight;
    }
}

