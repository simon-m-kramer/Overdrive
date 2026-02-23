// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboTemperatureWidget.h"
#include "Framework/TurboVehicle.h"
#include "Components/Image.h"

void UTurboTemperatureWidget::NativeConstruct()
{
    Super::NativeConstruct();
    CurrentNeedleAngle = NeedleAngleMin;
    SimulatedTemp = NormalTemperature;

    if (APawn* Pawn = GetOwningPlayerPawn())
    {
        CachedVehicle = Cast<ATurboVehicle>(Pawn);
    }
}

void UTurboTemperatureWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!CachedVehicle.IsValid())
    {
        return;
    }

    const float RPMAlpha = CachedVehicle->GetEngineRPM() / CachedVehicle->GetMaxEngineRPM();

    if (RPMAlpha > HotRPMThreshold)
    {
        const float HeatPressure = (RPMAlpha - HotRPMThreshold) / (1.0f - HotRPMThreshold);
        SimulatedTemp += HeatPressure * HeatRate * InDeltaTime;
    }
    else
    {
        SimulatedTemp = FMath::FInterpTo(SimulatedTemp, NormalTemperature, InDeltaTime, CoolRate);
    }

    SimulatedTemp = FMath::Clamp(SimulatedTemp, 0.0f, 1.0f);

    const float TargetAngle = FMath::Lerp(NeedleAngleMin, NeedleAngleMax, SimulatedTemp);
    CurrentNeedleAngle = FMath::FInterpTo(CurrentNeedleAngle, TargetAngle, InDeltaTime, NeedleInterpSpeed);

    if (Img_Needle)
    {
        Img_Needle->SetRenderTransformAngle(CurrentNeedleAngle);
    }
}