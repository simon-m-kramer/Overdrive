// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BifrostActionCharacter.generated.h"

class UBifrostActionStack;
class UBifrostAction;

UCLASS()
class BIFROST_API ABifrostActionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABifrostActionCharacter();

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

	UFUNCTION(BlueprintPure, Category = "Action")
	const TArray<UBifrostAction*>& GetActions() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	UBifrostActionStack* ActionStack;



};
