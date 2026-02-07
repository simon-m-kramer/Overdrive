// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAction_Apex.h"
#include "AI/TurboAIController.h"
#include "Framework/TurboVehicle.h"
#include "Framework/TurboRacingSpline.h"
#include "Framework/TurboGameplayTags.h"
#include "Components/SplineComponent.h"

UTurboAction_Apex::UTurboAction_Apex()
{
    ActionName = TEXT("Apex");
    ActionTag = TurboGameplayTags::Action_FollowPath;
}

void UTurboAction_Apex::Update(float DeltaTime)
{
    if (!Vehicle.IsValid() || !RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return;
    }

    // Get target and calculate steering — same as parent
    FVector TargetPoint = GetTargetPoint();
    CurrentSteeringInput = CalculateSteering(TargetPoint);
    Vehicle->SetSteeringInput(CurrentSteeringInput);

    // Determine corner phase
    CurrentPhase = DetermineCornerPhase();

    // Update exit throttle ramp
    if (CurrentPhase == EApexCornerPhase::Exit)
    {
        ExitThrottleAlpha = FMath::Clamp(ExitThrottleAlpha + DeltaTime * ExitThrottleRampRate, 0.0f, 1.0f);
    }
    else
    {
        ExitThrottleAlpha = 0.0f;
    }

    // Apply our speed control instead of parent's
    ApplySpeedControl();

    if (bDrawApexDebug)
    {
        FString PhaseStr;
        switch (CurrentPhase)
        {
        case EApexCornerPhase::Straight:  PhaseStr = TEXT("STRAIGHT"); break;
        case EApexCornerPhase::Approach:  PhaseStr = TEXT("APPROACH"); break;
        case EApexCornerPhase::TrailBrake: PhaseStr = TEXT("TRAIL BRAKE"); break;
        case EApexCornerPhase::Apex:      PhaseStr = TEXT("APEX"); break;
        case EApexCornerPhase::Exit:      PhaseStr = TEXT("EXIT"); break;
        }

        float AbsSteer = FMath::Abs(CurrentSteeringInput);
        GEngine->AddOnScreenDebugMessage(70, 0.0f, FColor::Orange,
            FString::Printf(TEXT("Apex [%s]: Steer=%.2f Curv=%.6f"),
                *PhaseStr, AbsSteer, GetCurrentCurvature()));
    }
}

void UTurboAction_Apex::ApplySpeedControl()
{
    if (!Vehicle.IsValid())
    {
        return;
    }

    float CurrentSpeed = Vehicle->GetSpeedKmh();
    float TargetSpeed = FindTargetSpeedAhead();
    float SpeedError = TargetSpeed - CurrentSpeed;
    float AbsSteering = FMath::Abs(CurrentSteeringInput);

    switch (CurrentPhase)
    {
    case EApexCornerPhase::Straight:
    {
        // Full commitment on straights
        if (SpeedError > 0.0f)
        {
            Vehicle->SetThrottleInput(1.0f);
            Vehicle->SetBrakeInput(0.0f);
        }
        else
        {
            float BrakeInput = FMath::Clamp(-SpeedError * BrakeProportionalGain, MinBrakeInput, MaxBrakeInput);
            Vehicle->SetThrottleInput(0.0f);
            Vehicle->SetBrakeInput(BrakeInput);
        }
        break;
    }

    case EApexCornerPhase::Approach:
    {
        // Hard braking in a straight line before the corner
        if (SpeedError < 0.0f)
        {
            float BrakeInput = FMath::Clamp(-SpeedError * BrakeProportionalGain * 1.5f, MinBrakeInput, MaxBrakeInput);
            Vehicle->SetThrottleInput(0.0f);
            Vehicle->SetBrakeInput(BrakeInput);
        }
        else
        {
            Vehicle->SetThrottleInput(1.0f);
            Vehicle->SetBrakeInput(0.0f);
        }
        break;
    }

    case EApexCornerPhase::TrailBrake:
    {
        // Core trail braking: release brake proportionally as steering increases
        // More steering = less brake, transferring load gradually
        float SteeringFactor = FMath::Clamp(AbsSteering * TrailBrakeReleaseFactor, 0.0f, 1.0f);
        float BrakeAmount = TrailBrakeMaxBrake * (1.0f - SteeringFactor);

        // Also factor in speed error — don't brake if already slow enough
        if (SpeedError > CoastingThresholdKmh)
        {
            BrakeAmount = 0.0f;
        }

        Vehicle->SetBrakeInput(FMath::Max(BrakeAmount, 0.0f));
        Vehicle->SetThrottleInput(TrailBrakeThrottle);
        break;
    }

    case EApexCornerPhase::Apex:
    {
        // At apex: minimal brake, begin feeding throttle
        if (SpeedError < -CoastingThresholdKmh)
        {
            // Still too fast at apex — light brake
            float BrakeInput = FMath::Clamp(-SpeedError * BrakeProportionalGain * 0.5f, 0.0f, 0.3f);
            Vehicle->SetThrottleInput(0.1f);
            Vehicle->SetBrakeInput(BrakeInput);
        }
        else
        {
            // At or below target — begin throttle
            float ThrottleInput = FMath::Clamp(0.3f + SpeedError * ThrottleProportionalGain, 0.2f, 0.6f);
            Vehicle->SetThrottleInput(ThrottleInput);
            Vehicle->SetBrakeInput(0.0f);
        }
        break;
    }

    case EApexCornerPhase::Exit:
    {
        // Progressive throttle application — ramp to full
        float ThrottleInput = FMath::Lerp(ExitMinThrottle, 1.0f, ExitThrottleAlpha);

        // Modulate by remaining steering — less throttle if still turning hard
        float SteeringPenalty = FMath::Clamp(AbsSteering * 0.5f, 0.0f, 0.3f);
        ThrottleInput = FMath::Max(ThrottleInput - SteeringPenalty, ExitMinThrottle);

        Vehicle->SetThrottleInput(ThrottleInput);
        Vehicle->SetBrakeInput(0.0f);
        break;
    }
    }

    Vehicle->SetHandbrakeInput(false);
}

float UTurboAction_Apex::FindTargetSpeedAhead() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return MaxSpeedKmh;
    }

    float LowestRequiredSpeed = MaxSpeedKmh;
    float CurrentDistance = AIController->GetCurrentSplineDistance();

    for (float Ahead = 0.0f; Ahead < ApexCornerScanDistance; Ahead += CornerScanInterval)
    {
        float ScanDist = CurrentDistance + Ahead;
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(ScanDist, SpeedCurvatureSampleRange);

        if (Curvature > KINDA_SMALL_NUMBER)
        {
            float RequiredSpeedCmPerSec = FMath::Sqrt(ApexGripFactor / Curvature);
            float RequiredSpeedKmh = RequiredSpeedCmPerSec * 0.036f;

            float DistanceFactor = Ahead / ApexCornerScanDistance;
            float AdjustedSpeed = RequiredSpeedKmh + (DistanceFactor * ApexDistanceSpeedBuffer);

            LowestRequiredSpeed = FMath::Min(LowestRequiredSpeed, AdjustedSpeed);
        }
    }

    return FMath::Clamp(LowestRequiredSpeed, MinCornerSpeedKmh, MaxSpeedKmh);
}

// =============================================================================
// PHASE DETECTION
// =============================================================================

EApexCornerPhase UTurboAction_Apex::DetermineCornerPhase() const
{
    float CurrentCurv = GetCurrentCurvature();
    float UpcomingCurv = GetUpcomingCurvature();
    float AbsSteering = FMath::Abs(CurrentSteeringInput);

    bool bInCorner = CurrentCurv > CornerCurvatureThreshold;
    bool bCornerAhead = UpcomingCurv > CornerCurvatureThreshold;
    bool bCurvatureIncreasing = IsCurvatureIncreasing();
    bool bCurvatureDecreasing = IsCurvatureDecreasing();

    if (!bInCorner && !bCornerAhead)
    {
        return EApexCornerPhase::Straight;
    }

    if (!bInCorner && bCornerAhead)
    {
        return EApexCornerPhase::Approach;
    }

    if (bInCorner && bCurvatureIncreasing)
    {
        return EApexCornerPhase::TrailBrake;
    }

    if (bInCorner && bCurvatureDecreasing)
    {
        return EApexCornerPhase::Exit;
    }

    return EApexCornerPhase::Apex;
}

float UTurboAction_Apex::GetCurrentCurvature() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return 0.0f;
    }

    return RacingSplineActor->GetCurvatureAtDistance(AIController->GetCurrentSplineDistance(), 300.0f);
}

float UTurboAction_Apex::GetUpcomingCurvature() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return 0.0f;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();

    float MaxCurvature = 0.0f;
    for (float Ahead = 200.0f; Ahead < ApproachScanDistance; Ahead += 200.0f)
    {
        float Curvature = RacingSplineActor->GetCurvatureAtDistance(CurrentDistance + Ahead, 300.0f);
        MaxCurvature = FMath::Max(MaxCurvature, Curvature);
    }

    return MaxCurvature;
}

bool UTurboAction_Apex::IsCurvatureIncreasing() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return false;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float CurvNow = RacingSplineActor->GetCurvatureAtDistance(CurrentDistance, 300.0f);
    float CurvAhead = RacingSplineActor->GetCurvatureAtDistance(CurrentDistance + 400.0f, 300.0f);

    return CurvAhead > CurvNow * 1.1f;
}

bool UTurboAction_Apex::IsCurvatureDecreasing() const
{
    if (!RacingSplineActor.IsValid() || !AIController.IsValid())
    {
        return false;
    }

    float CurrentDistance = AIController->GetCurrentSplineDistance();
    float CurvNow = RacingSplineActor->GetCurvatureAtDistance(CurrentDistance, 300.0f);
    float CurvAhead = RacingSplineActor->GetCurvatureAtDistance(CurrentDistance + 400.0f, 300.0f);

    return CurvAhead < CurvNow * 0.9f;
}
