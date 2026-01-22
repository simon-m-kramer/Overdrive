// Copyright Epic Games, Inc. All Rights Reserved.

#include "Starter/OverdriveWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UOverdriveWheelFront::UOverdriveWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}