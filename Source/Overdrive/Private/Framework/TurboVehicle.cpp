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

    // Chassis and glass — attached to root bone
    ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
    ChassisMesh->SetupAttachment(GetMesh(), FName("Root"));
    ChassisMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    ChassisMesh->SetCanEverAffectNavigation(false);

    GlassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassMesh"));
    GlassMesh->SetupAttachment(GetMesh(), FName("Root"));
    GlassMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    // Wheels — attached directly to physics bones
    WheelFL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFL"));
    WheelFL->SetupAttachment(GetMesh(), FName("Phys_Wheel_FL"));
    WheelFL->SetCollisionProfileName(TEXT("NoCollision"));
    WheelFL->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    WheelFR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFR"));
    WheelFR->SetupAttachment(GetMesh(), FName("Phys_Wheel_FR"));
    WheelFR->SetCollisionProfileName(TEXT("NoCollision"));
    WheelFR->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    WheelRL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRL"));
    WheelRL->SetupAttachment(GetMesh(), FName("Phys_Wheel_BL"));
    WheelRL->SetCollisionProfileName(TEXT("NoCollision"));
    WheelRL->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    WheelRR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRR"));
    WheelRR->SetupAttachment(GetMesh(), FName("Phys_Wheel_BR"));
    WheelRR->SetCollisionProfileName(TEXT("NoCollision"));
    WheelRR->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

    SetupWheels();
    SetupEngine();
    SetupTransmission();
    SetupSteering();

    // Detection Box
    DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
    DetectionBox->SetupAttachment(RootComponent);
    DetectionBox->SetBoxExtent(FVector(250.0f, 100.0f, 50.0f));
    DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionBox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
    DetectionBox->SetGenerateOverlapEvents(false);
    DetectionBox->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

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
    return FMath::Abs(VehicleMovement->GetForwardSpeed()) * 0.036f;
}

float ATurboVehicle::GetForwardSpeed() const
{
    return VehicleMovement->GetForwardSpeed();
}

void ATurboVehicle::SetupWheels()
{
    VehicleMovement->bLegacyWheelFrictionPosition = true;
    VehicleMovement->WheelSetups.SetNum(4);

    VehicleMovement->WheelSetups[0].WheelClass = UOverdriveSportsWheelFront::StaticClass();
    VehicleMovement->WheelSetups[0].BoneName = FName("Phys_Wheel_FL");
    VehicleMovement->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    VehicleMovement->WheelSetups[1].WheelClass = UOverdriveSportsWheelFront::StaticClass();
    VehicleMovement->WheelSetups[1].BoneName = FName("Phys_Wheel_FR");
    VehicleMovement->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    VehicleMovement->WheelSetups[2].WheelClass = UOverdriveSportsWheelRear::StaticClass();
    VehicleMovement->WheelSetups[2].BoneName = FName("Phys_Wheel_BL");
    VehicleMovement->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

    VehicleMovement->WheelSetups[3].WheelClass = UOverdriveSportsWheelRear::StaticClass();
    VehicleMovement->WheelSetups[3].BoneName = FName("Phys_Wheel_BR");
    VehicleMovement->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);
}

void ATurboVehicle::SetupEngine()
{
    VehicleMovement->EngineSetup.MaxTorque = 1000.0f;
    VehicleMovement->EngineSetup.MaxRPM = 7000.0f;
    VehicleMovement->EngineSetup.EngineIdleRPM = 900.0f;
    VehicleMovement->EngineSetup.EngineBrakeEffect = 0.2f;
    VehicleMovement->EngineSetup.EngineRevUpMOI = 5.0f;
    VehicleMovement->EngineSetup.EngineRevDownRate = 600.0f;

    FRichCurve* TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve();
    TorqueCurve->Reset();
    TorqueCurve->AddKey(0.0f, 0.5f);
    TorqueCurve->AddKey(0.25f, 0.8f);
    TorqueCurve->AddKey(0.5f, 1.0f);
    TorqueCurve->AddKey(0.75f, 0.9f);
    TorqueCurve->AddKey(1.0f, 0.7f);
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

    FRichCurve* SteeringCurve = VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve();
    SteeringCurve->Reset();
    SteeringCurve->AddKey(0.0f, 1.0f);
    SteeringCurve->AddKey(100.0f, 0.5f);
    SteeringCurve->AddKey(200.0f, 0.25f);
}