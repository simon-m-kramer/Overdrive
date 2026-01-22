// Copyright Epic Games, Inc. All Rights Reserved.

#include "Starter/OverdriveGameMode.h"
#include "Starter/OverdrivePlayerController.h"

AOverdriveGameMode::AOverdriveGameMode()
{
	PlayerControllerClass = AOverdrivePlayerController::StaticClass();
}
