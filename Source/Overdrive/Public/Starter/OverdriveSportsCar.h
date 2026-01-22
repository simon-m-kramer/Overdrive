// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OverdrivePawn.h"
#include "OverdriveSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class AOverdriveSportsCar : public AOverdrivePawn
{
	GENERATED_BODY()
	
public:

	AOverdriveSportsCar();
};
