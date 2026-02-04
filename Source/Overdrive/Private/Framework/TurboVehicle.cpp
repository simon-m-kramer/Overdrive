// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboVehicle.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Starter/OverdriveSportsWheelFront.h"
#include "Starter/OverdriveSportsWheelRear.h"
#include "Components/BoxComponent.h"
#include "Components/TurboVehicleDetectionComponent.h"

ATurboVehicle::ATurboVehicle()
{
    VehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(FName("Vehicle"));

    VehicleMovement->ChassisHeight = 144.0f;
    VehicleMovement->DragCoefficient = 0.31f;

    // Create components but attach to mesh root for now
    ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
    ChassisMesh->SetupAttachment(GetMesh());
    ChassisMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    ChassisMesh->SetCanEverAffectNavigation(false);

    GlassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassMesh"));
    GlassMesh->SetupAttachment(GetMesh());
    GlassMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    WheelFL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFL"));
    WheelFL->SetupAttachment(GetMesh());
    WheelFL->SetCollisionProfileName(TEXT("NoCollision"));

    WheelFR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFR"));
    WheelFR->SetupAttachment(GetMesh());
    WheelFR->SetCollisionProfileName(TEXT("NoCollision"));

    WheelRL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRL"));
    WheelRL->SetupAttachment(GetMesh());
    WheelRL->SetCollisionProfileName(TEXT("NoCollision"));

    WheelRR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRR"));
    WheelRR->SetupAttachment(GetMesh());
    WheelRR->SetCollisionProfileName(TEXT("NoCollision"));

    SetupWheels();
    SetupEngine();
    SetupTransmission();
    SetupSteering();

    // Detection Box
    DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
    DetectionBox->SetupAttachment(RootComponent);
    DetectionBox->SetBoxExtent(FVector(250.0f, 100.0f, 50.0f)); // Tune to car size
    DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionBox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
    DetectionBox->SetGenerateOverlapEvents(false);
    DetectionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f)); // Adjust Z as needed

    // Detection Component
    DetectionComponent = CreateDefaultSubobject<UTurboVehicleDetectionComponent>(TEXT("DetectionComponent"));

}

void ATurboVehicle::SetSteeringInput(float Value)
{
    VehicleMovement->SetSteeringInput(Value);
}

void ATurboVehicle::SetThrottleInput(float Value)
{
    VehicleMovement->SetThrottleInput(Value);
}

void ATurboVehicle::SetBrakeInput(float Value)
{
    VehicleMovement->SetBrakeInput(FMath::Clamp(Value, 0.f, 1.f));
}

void ATurboVehicle::SetHandbrakeInput(bool bEngaged)
{
    VehicleMovement->SetHandbrakeInput(bEngaged);
}

float ATurboVehicle::GetSpeedKmh() const
{
    return FMath::Abs(VehicleMovement->GetForwardSpeed()) * 0.036f; // cm/s to km/h
}

float ATurboVehicle::GetForwardSpeed() const
{
    return VehicleMovement->GetForwardSpeed();
}





void ATurboVehicle::SetupWheels()
{
    VehicleMovement->bLegacyWheelFrictionPosition = true;
    VehicleMovement->WheelSetups.SetNum(4);

    // Front left
    VehicleMovement->WheelSetups[0].WheelClass = UOverdriveSportsWheelFront::StaticClass();
    VehicleMovement->WheelSetups[0].BoneName = FName("Phys_Wheel_FL");
    VehicleMovement->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    // Front right
    VehicleMovement->WheelSetups[1].WheelClass = UOverdriveSportsWheelFront::StaticClass();
    VehicleMovement->WheelSetups[1].BoneName = FName("Phys_Wheel_FR");
    VehicleMovement->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    // Rear left
    VehicleMovement->WheelSetups[2].WheelClass = UOverdriveSportsWheelRear::StaticClass();
    VehicleMovement->WheelSetups[2].BoneName = FName("Phys_Wheel_BL");
    VehicleMovement->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    // Rear right
    VehicleMovement->WheelSetups[3].WheelClass = UOverdriveSportsWheelRear::StaticClass();
    VehicleMovement->WheelSetups[3].BoneName = FName("Phys_Wheel_BR");
    VehicleMovement->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);
}

void ATurboVehicle::SetupEngine()
{
    VehicleMovement->EngineSetup.MaxTorque = 750.0f;
    VehicleMovement->EngineSetup.MaxRPM = 7000.0f;
    VehicleMovement->EngineSetup.EngineIdleRPM = 900.0f;
    VehicleMovement->EngineSetup.EngineBrakeEffect = 0.2f;
    VehicleMovement->EngineSetup.EngineRevUpMOI = 5.0f;
    VehicleMovement->EngineSetup.EngineRevDownRate = 600.0f;

    // Torque curve - defines power output at different RPM percentages
    FRichCurve* TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve();
    TorqueCurve->Reset();
    TorqueCurve->AddKey(0.0f, 0.5f);      // 0% RPM -> 50% torque
    TorqueCurve->AddKey(0.25f, 0.8f);     // 25% RPM -> 80% torque
    TorqueCurve->AddKey(0.5f, 1.0f);      // 50% RPM -> 100% torque (peak)
    TorqueCurve->AddKey(0.75f, 0.9f);     // 75% RPM -> 90% torque
    TorqueCurve->AddKey(1.0f, 0.7f);      // 100% RPM -> 70% torque (falls off at redline)
}

void ATurboVehicle::SetupTransmission()
{
    VehicleMovement->TransmissionSetup.bUseAutomaticGears = true;
    VehicleMovement->TransmissionSetup.bUseAutoReverse = true;
    VehicleMovement->TransmissionSetup.FinalRatio = 2.81f;
    VehicleMovement->TransmissionSetup.ChangeUpRPM = 6000.0f;
    VehicleMovement->TransmissionSetup.ChangeDownRPM = 2000.0f;
    VehicleMovement->TransmissionSetup.GearChangeTime = 0.2f;
    VehicleMovement->TransmissionSetup.TransmissionEfficiency = 0.9f;

    VehicleMovement->TransmissionSetup.ForwardGearRatios.SetNum(5);
    VehicleMovement->TransmissionSetup.ForwardGearRatios[0] = 4.25f;
    VehicleMovement->TransmissionSetup.ForwardGearRatios[1] = 2.52f;
    VehicleMovement->TransmissionSetup.ForwardGearRatios[2] = 1.66f;
    VehicleMovement->TransmissionSetup.ForwardGearRatios[3] = 1.22f;
    VehicleMovement->TransmissionSetup.ForwardGearRatios[4] = 1.0f;

    VehicleMovement->TransmissionSetup.ReverseGearRatios.SetNum(1);
    VehicleMovement->TransmissionSetup.ReverseGearRatios[0] = 4.04f;
}

void ATurboVehicle::SetupSteering()
{
    VehicleMovement->SteeringSetup.SteeringType = ESteeringType::Ackermann;
    VehicleMovement->SteeringSetup.AngleRatio = 0.7f;

    // Steering curve - reduces steering angle at high speed
    FRichCurve* SteeringCurve = VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve();
    SteeringCurve->Reset();
    SteeringCurve->AddKey(0.0f, 1.0f);      // 0 km/h -> full steering
    SteeringCurve->AddKey(100.0f, 0.5f);    // 100 km/h -> 50% steering
    SteeringCurve->AddKey(200.0f, 0.25f);   // 200 km/h -> 25% steering
}





void ATurboVehicle::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    AttachWheelMeshes();
}

void ATurboVehicle::AttachWheelMeshes()
{
    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!SkelMesh)
    {
        return;
    }

    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);

    FRotator LeftWheelRotation(0.0f, -90.0f, 0.0f);
    FRotator RightWheelRotation(0.0f, 90.0f, 0.0f);

    if (WheelFL && SkelMesh->GetBoneIndex(FName("Phys_Wheel_FL")) != INDEX_NONE)
    {
        WheelFL->AttachToComponent(SkelMesh, AttachRules, FName("Phys_Wheel_FL"));
        WheelFL->SetRelativeRotation(LeftWheelRotation);
    }

    if (WheelFR && SkelMesh->GetBoneIndex(FName("Phys_Wheel_FR")) != INDEX_NONE)
    {
        WheelFR->AttachToComponent(SkelMesh, AttachRules, FName("Phys_Wheel_FR"));
        WheelFR->SetRelativeRotation(RightWheelRotation);
    }

    if (WheelRL && SkelMesh->GetBoneIndex(FName("Phys_Wheel_BL")) != INDEX_NONE)
    {
        WheelRL->AttachToComponent(SkelMesh, AttachRules, FName("Phys_Wheel_BL"));
        WheelRL->SetRelativeRotation(LeftWheelRotation);
    }

    if (WheelRR && SkelMesh->GetBoneIndex(FName("Phys_Wheel_BR")) != INDEX_NONE)
    {
        WheelRR->AttachToComponent(SkelMesh, AttachRules, FName("Phys_Wheel_BR"));
        WheelRR->SetRelativeRotation(RightWheelRotation);
    }

    if (GlassMesh && SkelMesh->GetBoneIndex(FName("Root")) != INDEX_NONE)
    {
        GlassMesh->AttachToComponent(SkelMesh, AttachRules, FName("Root"));
    }
}