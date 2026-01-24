// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TurboAIController.generated.h"

class UBifrostActionStack;
class UBifrostAction;
class ATurboRacingSpline;
class ATurboVehicle;

/**
 * 
 */
UCLASS()
class OVERDRIVE_API ATurboAIController : public AAIController
{
	GENERATED_BODY()
	

public:
	ATurboAIController();

	virtual void Tick(float DeltaTime) override;

	// =====================================================================
	// ACTION STACK
	// =====================================================================

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

	// =====================================================================
	// OTHER
	// =====================================================================

	UFUNCTION(BlueprintCallable, Category = "Spline")
	void FindRacingSpline();

	UFUNCTION(BlueprintPure, Category = "Spline")
	ATurboRacingSpline* GetRacingSplineActor() const { return RacingSplineActor; }


protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UBifrostActionStack> ActionStack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<ATurboRacingSpline> RacingSplineActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	TObjectPtr<ATurboVehicle> ControlledVehicle;

};
