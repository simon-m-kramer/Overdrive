// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BifrostAction.generated.h"

/**
 * 
 */
UCLASS()
class BIFROST_API UBifrostAction : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Start(bool bFirstTime) {}

	virtual void Update(float DeltaTime) {}

	virtual void End() {}

	virtual bool IsDone() { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FString ActionName = TEXT("Action");
};
