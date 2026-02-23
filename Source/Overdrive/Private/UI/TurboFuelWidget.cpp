// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboFuelWidget.h"
#include "Framework/TurboVehicle.h"
#include "Components/Image.h"

void UTurboFuelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    CurrentNeedleAngle = NeedleAngleMax; // Starts full
    FuelLevel = 1.0f;

    if (APawn* Pawn = GetOwningPlayerPawn())
    {
        CachedVehicle = Cast<ATurboVehicle>(Pawn);
    }
}

void UTurboFuelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!CachedVehicle.IsValid())
    {
        return;
    }

    const float RPMAlpha = CachedVehicle->GetEngineRPM() / CachedVehicle->GetMaxEngineRPM();
    const float Drain = BaseDrainRate * (1.0f + RPMAlpha * RPMDrainMultiplier) * InDeltaTime;
    FuelLevel = FMath::Max(FuelLevel - Drain, 0.05f);

    const float TargetAngle = FMath::Lerp(NeedleAngleMin, NeedleAngleMax, FuelLevel);
    CurrentNeedleAngle = FMath::FInterpTo(CurrentNeedleAngle, TargetAngle, InDeltaTime, NeedleInterpSpeed);

    if (Img_Needle)
    {
        Img_Needle->SetRenderTransformAngle(CurrentNeedleAngle);
    }
}