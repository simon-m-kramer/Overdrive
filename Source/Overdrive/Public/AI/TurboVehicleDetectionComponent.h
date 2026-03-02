// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurboVehicleDetectionComponent.generated.h"

class ATurboVehicle;

UENUM(BlueprintType)
enum class EOvertakeSide : uint8
{
    Left,
    Right
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVERDRIVE_API UTurboVehicleDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTurboVehicleDetectionComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // =========================================================================
    // QUERIES
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsCarAhead() const { return bCarAhead; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsCarOnLeft() const { return bCarOnLeft; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsCarOnRight() const { return bCarOnRight; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsCarBehind() const { return bCarBehind; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    float GetDistanceToCarAhead() const { return DistanceToCarAhead; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    float GetDistanceToCarBehind() const { return DistanceToCarBehind; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    ATurboVehicle* GetCarAhead() const { return CarAhead; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    ATurboVehicle* GetCarBehind() const { return CarBehind; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsOvertakeSafe(EOvertakeSide Side) const;

    UFUNCTION(BlueprintPure, Category = "Detection")
    ATurboVehicle* GetCarOnLeft() const { return CarOnLeft; }

    UFUNCTION(BlueprintPure, Category = "Detection")
    ATurboVehicle* GetCarOnRight() const { return CarOnRight; }

    // =========================================================================
    // CONFIGURATION - FORWARD
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Detection|Forward")
    float ForwardDetectionRange = 3000.0f;

    /** Half-width of forward detection box (lateral coverage) */
    UPROPERTY(EditAnywhere, Category = "Detection|Forward")
    float ForwardBoxHalfWidth = 300.0f;

    /** Half-height of forward detection box */
    UPROPERTY(EditAnywhere, Category = "Detection|Forward")
    float ForwardBoxHalfHeight = 100.0f;

    // =========================================================================
    // CONFIGURATION - BEHIND
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Detection|Behind")
    float BehindDetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Detection|Behind")
    float BehindTraceRadius = 150.0f;

    // =========================================================================
    // CONFIGURATION - SIDE ZONES
    // =========================================================================

    /** How far sideways the side box extends */
    UPROPERTY(EditAnywhere, Category = "Detection|Side")
    float SideDetectionWidth = 300.0f;

    /** Length of side box along car's forward axis (roughly car length) */
    UPROPERTY(EditAnywhere, Category = "Detection|Side")
    float SideDetectionLength = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Detection|Side")
    float SideDetectionHeight = 100.0f;

    /** How far sideways from car center to place the box center */
    UPROPERTY(EditAnywhere, Category = "Detection|Side")
    float SideDetectionOffset = 150.0f;

    /** Forward offset of side box center relative to car center (negative = shifted back) */
    UPROPERTY(EditAnywhere, Category = "Detection|Side")
    float SideDetectionForwardOffset = 25.0f;

    // =========================================================================
    // DEBUG
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebug = false;

protected:
    virtual void BeginPlay() override;

private:
    void UpdateForwardDetection();
    void UpdateBehindDetection();
    void UpdateSideDetection();

    UPROPERTY()
    TObjectPtr<ATurboVehicle> OwnerVehicle;

    UPROPERTY()
    TObjectPtr<ATurboVehicle> CarOnLeft;

    UPROPERTY()
    TObjectPtr<ATurboVehicle> CarOnRight;

    // Detection results
    bool bCarAhead = false;
    bool bCarOnLeft = false;
    bool bCarOnRight = false;
    bool bCarBehind = false;

    float DistanceToCarAhead = 0.0f;
    float DistanceToCarBehind = 0.0f;

    UPROPERTY()
    TObjectPtr<ATurboVehicle> CarAhead;

    UPROPERTY()
    TObjectPtr<ATurboVehicle> CarBehind;

    // Collision channel
    ECollisionChannel DetectionChannel = ECC_GameTraceChannel1;
};
