// Copyright Simon Kramer. All Rights Reserved.


#include "Framework/TurboVehicle.h"
#include "Framework/TurboVehicleData.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Starter/OverdriveSportsWheelFront.h"
#include "Starter/OverdriveSportsWheelRear.h"
#include "Components/BoxComponent.h"
#include "AI/TurboVehicleDetectionComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"


// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------

ATurboVehicle::ATurboVehicle()
{
	VehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// Chassis and glass — meshes assigned later from data asset
	ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
	ChassisMesh->SetupAttachment(GetMesh(), FName("Root"));
	ChassisMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ChassisMesh->SetCanEverAffectNavigation(false);

	GlassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassMesh"));
	GlassMesh->SetupAttachment(GetMesh(), FName("Root"));
	GlassMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	// Pre-create four wheel mesh components — bone attachments set later
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

	WheelMeshComponents = { WheelFL, WheelFR, WheelRL, WheelRR };

	// Chaos needs a valid wheel topology at construction time.
	// These defaults get the physics working even without a data asset.
	SetupDefaultWheels();
	SetupDefaultEngine();
	SetupDefaultTransmission();
	SetupDefaultSteering();

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


// -----------------------------------------------------------------------
// OnConstruction — runs in-editor when you place or modify the actor.
// Handles visual-only setup so you get a preview in the viewport.
// -----------------------------------------------------------------------

void ATurboVehicle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!VehicleData)
	{
		return;
	}

	VehicleName = VehicleData->VehicleName;
	ApplyMeshes();
	ApplyWheelMeshes();
}


// -----------------------------------------------------------------------
// PostEditChangeProperty — re-run visuals when the data asset is swapped
// in the details panel.
// -----------------------------------------------------------------------

#if WITH_EDITOR
void ATurboVehicle::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATurboVehicle, VehicleData))
	{
		OnConstruction(GetActorTransform());
	}
}
#endif


// -----------------------------------------------------------------------
// PostInitializeComponents — called after components are initialized
// but before BeginPlay. Properties have been deserialized so VehicleData
// is available. This is where we apply engine, transmission, steering.
// Wheel topology is already locked from the constructor.
// -----------------------------------------------------------------------

void ATurboVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!VehicleData)
	{
		return;
	}

	VehicleName = VehicleData->VehicleName;
	VehicleMovement->ChassisHeight = VehicleData->ChassisHeight;
	VehicleMovement->DragCoefficient = VehicleData->DragCoefficient;

	ApplyMeshes();
	ApplyWheelMeshes();
	ApplyEngine();
	ApplyTransmission();
	ApplySteering();
}


// -----------------------------------------------------------------------
// Constructor-time defaults
// -----------------------------------------------------------------------

void ATurboVehicle::SetupDefaultWheels()
{
	VehicleMovement->bLegacyWheelFrictionPosition = true;
	VehicleMovement->WheelSetups.SetNum(4);

	VehicleMovement->WheelSetups[0].WheelClass = UOverdriveSportsWheelFront::StaticClass();
	VehicleMovement->WheelSetups[0].BoneName = FName("Phys_Wheel_FL");
	VehicleMovement->WheelSetups[0].AdditionalOffset = FVector::ZeroVector;

	VehicleMovement->WheelSetups[1].WheelClass = UOverdriveSportsWheelFront::StaticClass();
	VehicleMovement->WheelSetups[1].BoneName = FName("Phys_Wheel_FR");
	VehicleMovement->WheelSetups[1].AdditionalOffset = FVector::ZeroVector;

	VehicleMovement->WheelSetups[2].WheelClass = UOverdriveSportsWheelRear::StaticClass();
	VehicleMovement->WheelSetups[2].BoneName = FName("Phys_Wheel_BL");
	VehicleMovement->WheelSetups[2].AdditionalOffset = FVector::ZeroVector;

	VehicleMovement->WheelSetups[3].WheelClass = UOverdriveSportsWheelRear::StaticClass();
	VehicleMovement->WheelSetups[3].BoneName = FName("Phys_Wheel_BR");
	VehicleMovement->WheelSetups[3].AdditionalOffset = FVector::ZeroVector;
}

void ATurboVehicle::SetupDefaultEngine()
{
	VehicleMovement->EngineSetup.MaxTorque = 750.0f;
	VehicleMovement->EngineSetup.MaxRPM = 7000.0f;
	VehicleMovement->EngineSetup.EngineIdleRPM = 900.0f;
	VehicleMovement->EngineSetup.EngineBrakeEffect = 0.2f;
	VehicleMovement->EngineSetup.EngineRevUpMOI = 5.0f;
	VehicleMovement->EngineSetup.EngineRevDownRate = 600.0f;

	FRichCurve* TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve();
	TorqueCurve->Reset();
	TorqueCurve->AddKey(1000.0f, 0.5f);
	TorqueCurve->AddKey(1800.0f, 0.9f);
	TorqueCurve->AddKey(3500.0f, 1.0f);
	TorqueCurve->AddKey(6000.0f, 0.8f);
	TorqueCurve->AddKey(7000.0f, 0.0f);
}

void ATurboVehicle::SetupDefaultTransmission()
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

void ATurboVehicle::SetupDefaultSteering()
{
	VehicleMovement->SteeringSetup.SteeringType = ESteeringType::Ackermann;
	VehicleMovement->SteeringSetup.AngleRatio = 0.7f;

	FRichCurve* SteeringCurve = VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve();
	SteeringCurve->Reset();
	SteeringCurve->AddKey(0.0f, 1.0f);
	SteeringCurve->AddKey(100.0f, 0.5f);
	SteeringCurve->AddKey(200.0f, 0.25f);
}


// -----------------------------------------------------------------------
// Data asset application — meshes and visuals
// -----------------------------------------------------------------------

void ATurboVehicle::ApplyMeshes()
{
	if (!VehicleData)
	{
		return;
	}

	// Skeletal mesh (the physics body)
	if (!VehicleData->VehicleMesh.IsNull())
	{
		USkeletalMesh* SkelMesh = VehicleData->VehicleMesh.LoadSynchronous();
		if (SkelMesh)
		{
			GetMesh()->SetSkeletalMesh(SkelMesh);
		}
	}

	if (VehicleData->AnimBlueprintClass)
	{
		GetMesh()->SetAnimInstanceClass(VehicleData->AnimBlueprintClass);
	}

	// Optional chassis
	if (VehicleData->UsesSeparateChassisMesh())
	{
		UStaticMesh* ChassisAsset = VehicleData->ChassisMesh.LoadSynchronous();
		if (ChassisAsset)
		{
			ChassisMesh->SetStaticMesh(ChassisAsset);
			ChassisMesh->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				VehicleData->ChassisBoneName
			);
			ChassisMesh->SetVisibility(true);
		}
	}
	else
	{
		ChassisMesh->SetStaticMesh(nullptr);
		ChassisMesh->SetVisibility(false);
	}

	// Optional glass
	if (VehicleData->UsesSeparateGlassMesh())
	{
		UStaticMesh* GlassAsset = VehicleData->GlassMesh.LoadSynchronous();
		if (GlassAsset)
		{
			GlassMesh->SetStaticMesh(GlassAsset);
			GlassMesh->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				VehicleData->GlassBoneName
			);
			GlassMesh->SetVisibility(true);
		}
	}
	else
	{
		GlassMesh->SetStaticMesh(nullptr);
		GlassMesh->SetVisibility(false);
	}
}

void ATurboVehicle::ApplyWheelMeshes()
{
	if (!VehicleData)
	{
		return;
	}

	const int32 NumWheels = FMath::Min(VehicleData->Wheels.Num(), WheelMeshComponents.Num());

	for (int32 i = 0; i < NumWheels; ++i)
	{
		const FTurboWheelSetup& Src = VehicleData->Wheels[i];
		UStaticMeshComponent* WheelComp = WheelMeshComponents[i];

		// Attach to the correct bone
		WheelComp->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			Src.BoneName
		);

		// If the data asset specifies a wheel mesh, load and assign it
		if (!Src.WheelMesh.IsNull())
		{
			UStaticMesh* WheelAsset = Src.WheelMesh.LoadSynchronous();
			if (WheelAsset)
			{
				WheelComp->SetStaticMesh(WheelAsset);
			}
			WheelComp->SetRelativeRotation(Src.WheelMeshRotation);
			WheelComp->SetVisibility(true);
		}
		else
		{
			// Wheels are baked into the skeletal mesh — hide the empty component
			WheelComp->SetStaticMesh(nullptr);
			WheelComp->SetVisibility(false);
		}
	}

	// Hide any extra pre-created components if data asset has fewer than 4 wheels
	for (int32 i = NumWheels; i < WheelMeshComponents.Num(); ++i)
	{
		WheelMeshComponents[i]->SetStaticMesh(nullptr);
		WheelMeshComponents[i]->SetVisibility(false);
	}
}


// -----------------------------------------------------------------------
// Data asset application — mechanical (engine, transmission, steering)
// -----------------------------------------------------------------------

void ATurboVehicle::ApplyEngine()
{
	if (!VehicleData)
	{
		return;
	}

	const FTurboEngineSetup& Eng = VehicleData->Engine;

	VehicleMovement->EngineSetup.MaxTorque = Eng.MaxTorque;
	VehicleMovement->EngineSetup.MaxRPM = Eng.MaxRPM;
	VehicleMovement->EngineSetup.EngineIdleRPM = Eng.IdleRPM;
	VehicleMovement->EngineSetup.EngineBrakeEffect = Eng.EngineBrakeEffect;
	VehicleMovement->EngineSetup.EngineRevUpMOI = Eng.EngineRevUpMOI;
	VehicleMovement->EngineSetup.EngineRevDownRate = Eng.EngineRevDownRate;

	FRichCurve* TorqueCurve = VehicleMovement->EngineSetup.TorqueCurve.GetRichCurve();
	TorqueCurve->Reset();

	if (Eng.TorqueCurveKeys.Num() > 0)
	{
		for (const FVector2D& Key : Eng.TorqueCurveKeys)
		{
			TorqueCurve->AddKey(Key.X, Key.Y);
		}
	}
	else
	{
		// Sensible default curve
		TorqueCurve->AddKey(1000.0f, 0.5f);
		TorqueCurve->AddKey(1800.0f, 0.9f);
		TorqueCurve->AddKey(3500.0f, 1.0f);
		TorqueCurve->AddKey(6000.0f, 0.8f);
		TorqueCurve->AddKey(Eng.MaxRPM, 0.0f);
	}
}

void ATurboVehicle::ApplyTransmission()
{
	if (!VehicleData)
	{
		return;
	}

	const FTurboTransmissionSetup& Trans = VehicleData->Transmission;

	VehicleMovement->TransmissionSetup.bUseAutomaticGears = Trans.bAutomaticGears;
	VehicleMovement->TransmissionSetup.bUseAutoReverse = Trans.bAutoReverse;
	VehicleMovement->TransmissionSetup.FinalRatio = Trans.FinalRatio;
	VehicleMovement->TransmissionSetup.ChangeUpRPM = Trans.ChangeUpRPM;
	VehicleMovement->TransmissionSetup.ChangeDownRPM = Trans.ChangeDownRPM;
	VehicleMovement->TransmissionSetup.GearChangeTime = Trans.GearChangeTime;
	VehicleMovement->TransmissionSetup.TransmissionEfficiency = Trans.TransmissionEfficiency;

	if (Trans.ForwardGearRatios.Num() > 0)
	{
		VehicleMovement->TransmissionSetup.ForwardGearRatios = Trans.ForwardGearRatios;
	}

	if (Trans.ReverseGearRatios.Num() > 0)
	{
		VehicleMovement->TransmissionSetup.ReverseGearRatios = Trans.ReverseGearRatios;
	}
}

void ATurboVehicle::ApplySteering()
{
	if (!VehicleData)
	{
		return;
	}

	const FTurboSteeringSetup& Steer = VehicleData->Steering;

	VehicleMovement->SteeringSetup.SteeringType = ESteeringType::Ackermann;
	VehicleMovement->SteeringSetup.AngleRatio = Steer.AngleRatio;

	FRichCurve* SteeringCurve = VehicleMovement->SteeringSetup.SteeringCurve.GetRichCurve();
	SteeringCurve->Reset();

	if (Steer.SteeringCurveKeys.Num() > 0)
	{
		for (const FVector2D& Key : Steer.SteeringCurveKeys)
		{
			SteeringCurve->AddKey(Key.X, Key.Y);
		}
	}
	else
	{
		SteeringCurve->AddKey(0.0f, 1.0f);
		SteeringCurve->AddKey(100.0f, 0.5f);
		SteeringCurve->AddKey(200.0f, 0.25f);
	}
}


// -----------------------------------------------------------------------
// Input & query functions
// -----------------------------------------------------------------------

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

int32 ATurboVehicle::GetCurrentGear() const
{
	return VehicleMovement->GetCurrentGear();
}

float ATurboVehicle::GetEngineRPM() const
{
	return VehicleMovement->GetEngineRotationSpeed();
}

float ATurboVehicle::GetMaxEngineRPM() const
{
	return VehicleMovement->EngineSetup.MaxRPM;
}
