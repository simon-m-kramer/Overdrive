// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BifrostCharacter.generated.h"

class UBifrostActionStack;
class UBifrostAction;

UCLASS()
class BIFROST_API ABifrostCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABifrostCharacter();

	virtual void Tick(float DeltaTime) override;

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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	UBifrostActionStack* ActionStack;



};
