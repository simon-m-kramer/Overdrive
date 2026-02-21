// Copyright Simon Kramer. All Rights Reserved.


#include "UI/TurboCompassWidget.h"
#include "Framework/TurboVehicle.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UTurboCompassWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		CachedVehicle = Cast<ATurboVehicle>(Pawn);
	}

	if (Img_Compass && CompassMaterial)
	{
		CompassMID = UMaterialInstanceDynamic::Create(CompassMaterial, this);
		Img_Compass->SetBrushFromMaterial(CompassMID);
	}
}

void UTurboCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedVehicle.IsValid() || !CompassMID)
	{
		return;
	}

	const float Yaw = CachedVehicle->GetActorRotation().Yaw;

	// Normalize to 0-1 range for UV offset
	// Yaw goes -180 to 180, we need 0 to 1
	const float Offset = FMath::Fmod(Yaw / 360.0f + 1.0f, 1.0f);

	CompassMID->SetScalarParameterValue(FName("Offset"), Offset);
}