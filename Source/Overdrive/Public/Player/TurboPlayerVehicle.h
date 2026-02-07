// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/TurboVehicle.h"
#include "TurboPlayerVehicle.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
struct FInputActionValue;

UCLASS(abstract)
class OVERDRIVE_API ATurboPlayerVehicle : public ATurboVehicle
{
    GENERATED_BODY()

public:
    ATurboPlayerVehicle();

    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

protected:
    // =========================================================================
    // CAMERAS
    // =========================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> FrontSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FrontCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> BackSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> BackCamera;

    bool bFrontCameraActive = false;

    // =========================================================================
    // INPUT ACTIONS
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SteeringAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> BrakeAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> HandbrakeAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> LookAroundAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> ToggleCameraAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> ResetVehicleAction;

    // =========================================================================
    // FLIP CHECK
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Flip Check", meta = (Units = "s"))
    float FlipCheckTime = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Flip Check")
    float FlipCheckMinDot = -0.2f;

    FTimerHandle FlipCheckTimer;
    bool bPreviousFlipCheck = false;

    // =========================================================================
    // INPUT HANDLERS
    // =========================================================================

    void OnSteering(const FInputActionValue& Value);
    void OnThrottle(const FInputActionValue& Value);
    void OnBrake(const FInputActionValue& Value);
    void OnBrakeStarted(const FInputActionValue& Value);
    void OnBrakeCompleted(const FInputActionValue& Value);
    void OnHandbrakeStarted(const FInputActionValue& Value);
    void OnHandbrakeCompleted(const FInputActionValue& Value);
    void OnLookAround(const FInputActionValue& Value);
    void OnToggleCamera(const FInputActionValue& Value);
    void OnResetVehicle(const FInputActionValue& Value);

    UFUNCTION()
    void FlippedCheck();

    void ResetVehicleTransform();

    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
    void BrakeLights(bool bBraking);

public:
    FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
    FORCEINLINE UCameraComponent* GetFrontCamera() const { return FrontCamera; }
    FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
    FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
};
