// Copyright Epic Games, Inc. All Rights Reserved.

#include "Starter/OverdriveWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UOverdriveWheelRear::UOverdriveWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}