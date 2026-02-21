// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboTachometerWidget.h"
#include "Framework/TurboVehicle.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UTurboTachometerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentNeedleAngle = NeedleAngleMin;

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		CachedVehicle = Cast<ATurboVehicle>(Pawn);
	}
}

void UTurboTachometerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedVehicle.IsValid())
	{
		return;
	}

	const float RPM = CachedVehicle->GetEngineRPM();
	const float MaxRPM = CachedVehicle->GetMaxEngineRPM();
	const float Alpha = FMath::Clamp(RPM / MaxRPM, 0.0f, 1.0f);
	const float TargetAngle = FMath::Lerp(NeedleAngleMin, NeedleAngleMax, Alpha);

	CurrentNeedleAngle = FMath::FInterpTo(CurrentNeedleAngle, TargetAngle, InDeltaTime, NeedleInterpSpeed);

	if (Img_Needle)
	{
		Img_Needle->SetRenderTransformAngle(CurrentNeedleAngle);
	}

	if (Txt_Gear)
	{
		Txt_Gear->SetText(FText::FromString(GearToString(CachedVehicle->GetCurrentGear())));
	}
}

FString UTurboTachometerWidget::GearToString(int32 Gear) const
{
	if (Gear < 0) return TEXT("R");
	if (Gear == 0) return TEXT("N");
	return FString::FromInt(Gear);
}