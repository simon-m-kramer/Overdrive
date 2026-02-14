// Copyright Simon Kramer. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "TurboRacingSpline.generated.h"

class USplineComponent;

UCLASS()
class OVERDRIVE_API ATurboRacingSpline : public AActor
{
    GENERATED_BODY()

public:
    ATurboRacingSpline();

    USplineComponent* GetSplineComponent() const { return Spline; }
    const FGameplayTagContainer& GetGameplayTags() const { return GameplayTags; }

    // =========================================================================
    // RACING LINE
    // =========================================================================

    UFUNCTION(BlueprintCallable, Category = "Racing Line")
    void CalculateRacingLine();

    UFUNCTION(BlueprintPure, Category = "Racing Line")
    float GetRacingLineOffset(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "Racing Line")
    FVector GetPointOnRacingLine(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "Racing Line")
    bool IsRacingLineReady() const { return bRacingLineCalculated; }

    void DrawDebugRacingLine(UWorld* World) const;

    // =========================================================================
    // CURVATURE ANALYSIS
    // =========================================================================

    UFUNCTION(BlueprintPure, Category = "Curvature")
    float GetCurvatureAtDistance(float Distance, float SampleRange = 300.0f) const;

    UFUNCTION(BlueprintPure, Category = "Curvature")
    float GetTurnSign(float Distance, float InLookaheadDistance = 200.0f) const;

    // =========================================================================
    // TRACK CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track")
    float TrackWidth = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TrackWidthUsage = 0.85f;

    // =========================================================================
    // RACING LINE CONFIGURATION
    // =========================================================================

    UPROPERTY(EditAnywhere, Category = "Racing Line|Smoothing")
    int32 SmoothingPasses = 0;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Smoothing")
    int32 SmoothingWindowMin = 15;

    UPROPERTY(EditAnywhere, Category = "Racing Line|Smoothing")
    int32 SmoothingWindowMax = 40;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float MinCurvatureThreshold = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float CurvatureSampleRange = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float RacingLineLookahead = 10000.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float LookaheadStepSize = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float TurnSignLookahead = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Racing Line")
    float RacingLineSampleInterval = 100.0f;



protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    TObjectPtr<USplineComponent> Spline;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
    FGameplayTagContainer GameplayTags;

private:
    float CalculateIdealOffset(float Distance) const;
    float WrapDistance(float Distance) const;

    TArray<float> PreCalculatedOffsets;
    float MaxTrackCurvature = 0.0f;
    bool bRacingLineCalculated = false;

    void SmoothRacingLine();

};
