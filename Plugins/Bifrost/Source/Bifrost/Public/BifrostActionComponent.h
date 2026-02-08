// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BifrostActionComponent.generated.h"

class UBifrostActionStack;
class UBifrostAction;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BIFROST_API UBifrostActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBifrostActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PushAction(UBifrostAction* NewAction);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void RemoveAction(UBifrostAction* InAction);

	UFUNCTION(BlueprintPure, Category = "Action")
	bool Contains(UBifrostAction* InAction);

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsEmpty() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	UBifrostAction* GetCurrentAction() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	const TArray<UBifrostAction*>& GetActions() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	UBifrostActionStack* ActionStack;
};
