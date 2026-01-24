// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TurboVehicleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTurboVehicleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class OVERDRIVE_API ITurboVehicleInterface
{
	GENERATED_BODY()

public:
	// =====================================================================
	// ACTUATORS (Inputs)
	// =====================================================================

	/** Set steering: -1.0 (Left) to 1.0 (Right) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Control")
	void SetSteeringInput(float Value);

	/** Set throttle: 0.0 to 1.0 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Control")
	void SetThrottleInput(float Value);

	/** Set brake: 0.0 to 1.0 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Control")
	void SetBrakeInput(float Value);

	/** Engages or disengages the handbrake */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Control")
	void SetHandbrakeInput(bool bEngaged);

	// =====================================================================
	// SENSORS (Data for AI Decision Making)
	// =====================================================================

	/** Returns current speed in Kilometers per Hour */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Stats")
	float GetSpeedKmh() const;

	/** Returns speed in local forward direction (negative if reversing) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Stats")
	float GetForwardSpeed() const;

	/** Returns a 0.0 to 1.0 value representing how much the wheels are slipping */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Stats")
	float GetMaxWheelSlip() const;

	// =====================================================================
	// NAVIGATION HELPERS
	// =====================================================================

	/** Returns the world-space location where the car is 'aiming' (Front Bumper) */
	// This helps AI steer toward a path point more accurately than using ActorLocation.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Turbo|Vehicle Navigation")
	FVector GetVehicleFrontLocation() const;
};
