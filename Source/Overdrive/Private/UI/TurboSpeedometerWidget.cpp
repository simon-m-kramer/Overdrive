// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboSpeedometerWidget.h"
#include "Framework/TurboVehicle.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"


void UTurboSpeedometerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentNeedleAngle = NeedleAngleMin;

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		CachedVehicle = Cast<ATurboVehicle>(Pawn);
	}

}

void UTurboSpeedometerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedVehicle.IsValid())
	{
		return;
	}

	const float ClampedSpeed = FMath::Clamp(CachedVehicle->GetSpeedKmh(), 0.0f, MaxDisplaySpeed);
	const float Alpha = ClampedSpeed / MaxDisplaySpeed;
	const float TargetAngle = FMath::Lerp(NeedleAngleMin, NeedleAngleMax, Alpha);

	CurrentNeedleAngle = FMath::FInterpTo(CurrentNeedleAngle, TargetAngle, InDeltaTime, NeedleInterpSpeed);

	if (Img_Needle)
	{
		Img_Needle->SetRenderTransformAngle(CurrentNeedleAngle);
	}

	if (Txt_Speed)
	{
		Txt_Speed->SetText(FText::AsNumber(FMath::RoundToInt(ClampedSpeed)));
	}
}