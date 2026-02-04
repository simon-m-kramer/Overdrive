// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BifrostActionStack.generated.h"


class UBifrostAction;
/**
 * 
 */
UCLASS()
class BIFROST_API UBifrostActionStack : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Action")
	void UpdateActions(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PushAction(UBifrostAction* NewAction);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void RemoveAction(UBifrostAction* InAction);

	UFUNCTION(BlueprintPure, Category = "Action")
	bool Contains(UBifrostAction* InAction);

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsEmpty() const { return !CurrentAction && ActionStack.Num() == 0; }

	UFUNCTION(BlueprintPure, Category = "Action")
	UBifrostAction* GetCurrentAction() const { return CurrentAction; }

	UFUNCTION(BlueprintPure, Category = "Action")
	const TArray<UBifrostAction*>& GetActions() const { return ActionStack; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TArray<UBifrostAction*> ActionStack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TSet<UBifrostAction*> FirstTimeActions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	UBifrostAction* CurrentAction;
};
