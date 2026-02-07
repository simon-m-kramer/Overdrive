// Copyright Simon Kramer. All Rights Reserved.


#include "Player/TurboPlayerVehicle.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TimerManager.h"

ATurboPlayerVehicle::ATurboPlayerVehicle()
{
    // Front camera
    FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FrontSpringArm"));
    FrontSpringArm->SetupAttachment(GetMesh());
    FrontSpringArm->TargetArmLength = 0.0f;
    FrontSpringArm->bDoCollisionTest = false;
    FrontSpringArm->bEnableCameraRotationLag = true;
    FrontSpringArm->CameraRotationLagSpeed = 15.0f;
    FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

    FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrontCamera"));
    FrontCamera->SetupAttachment(FrontSpringArm);
    FrontCamera->bAutoActivate = false;

    // Back camera
    BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("BackSpringArm"));
    BackSpringArm->SetupAttachment(GetMesh());
    BackSpringArm->TargetArmLength = 650.0f;
    BackSpringArm->SocketOffset.Z = 150.0f;
    BackSpringArm->bDoCollisionTest = false;
    BackSpringArm->bInheritPitch = false;
    BackSpringArm->bInheritRoll = false;
    BackSpringArm->bEnableCameraRotationLag = true;
    BackSpringArm->CameraRotationLagSpeed = 2.0f;
    BackSpringArm->CameraLagMaxDistance = 50.0f;

    BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BackCamera"));
    BackCamera->SetupAttachment(BackSpringArm);
}

void ATurboPlayerVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        Input->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnSteering);
        Input->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ATurboPlayerVehicle::OnSteering);

        Input->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnThrottle);
        Input->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ATurboPlayerVehicle::OnThrottle);

        Input->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnBrake);
        Input->BindAction(BrakeAction, ETriggerEvent::Started, this, &ATurboPlayerVehicle::OnBrakeStarted);
        Input->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ATurboPlayerVehicle::OnBrakeCompleted);

        Input->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATurboPlayerVehicle::OnHandbrakeStarted);
        Input->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATurboPlayerVehicle::OnHandbrakeCompleted);

        Input->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnLookAround);
        Input->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnToggleCamera);
        Input->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ATurboPlayerVehicle::OnResetVehicle);
    }
}

void ATurboPlayerVehicle::BeginPlay()
{
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(FlipCheckTimer, this, &ATurboPlayerVehicle::FlippedCheck, FlipCheckTime, true);
}

void ATurboPlayerVehicle::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(FlipCheckTimer);

    Super::EndPlay(EndPlayReason);
}

void ATurboPlayerVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Angular damping in midair
    bool bOnGround = VehicleMovement->IsMovingOnGround();
    GetMesh()->SetAngularDamping(bOnGround ? 0.0f : 3.0f);

    // Smooth camera yaw back to center
    float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
    CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, DeltaTime, 1.0f);
    BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

// =============================================================================
// INPUT HANDLERS
// =============================================================================

void ATurboPlayerVehicle::OnSteering(const FInputActionValue& Value)
{
    SetSteeringInput(Value.Get<float>());
}

void ATurboPlayerVehicle::OnThrottle(const FInputActionValue& Value)
{
    SetThrottleInput(Value.Get<float>());
    SetBrakeInput(0.0f);
}

void ATurboPlayerVehicle::OnBrake(const FInputActionValue& Value)
{
    SetBrakeInput(Value.Get<float>());
    SetThrottleInput(0.0f);
}

void ATurboPlayerVehicle::OnBrakeStarted(const FInputActionValue& Value)
{
    BrakeLights(true);
}

void ATurboPlayerVehicle::OnBrakeCompleted(const FInputActionValue& Value)
{
    BrakeLights(false);
    SetBrakeInput(0.0f);
}

void ATurboPlayerVehicle::OnHandbrakeStarted(const FInputActionValue& Value)
{
    SetHandbrakeInput(true);
    BrakeLights(true);
}

void ATurboPlayerVehicle::OnHandbrakeCompleted(const FInputActionValue& Value)
{
    SetHandbrakeInput(false);
    BrakeLights(false);
}

void ATurboPlayerVehicle::OnLookAround(const FInputActionValue& Value)
{
    BackSpringArm->AddLocalRotation(FRotator(0.0f, Value.Get<float>(), 0.0f));
}

void ATurboPlayerVehicle::OnToggleCamera(const FInputActionValue& Value)
{
    bFrontCameraActive = !bFrontCameraActive;
    FrontCamera->SetActive(bFrontCameraActive);
    BackCamera->SetActive(!bFrontCameraActive);
}

void ATurboPlayerVehicle::OnResetVehicle(const FInputActionValue& Value)
{
    ResetVehicleTransform();
}

// =============================================================================
// UTILITY
// =============================================================================

void ATurboPlayerVehicle::ResetVehicleTransform()
{
    FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

    FRotator ResetRotation = GetActorRotation();
    ResetRotation.Pitch = 0.0f;
    ResetRotation.Roll = 0.0f;

    SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

    GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void ATurboPlayerVehicle::FlippedCheck()
{
    const float UpDot = FVector::DotProduct(FVector::UpVector, GetMesh()->GetUpVector());

    if (UpDot < FlipCheckMinDot)
    {
        if (bPreviousFlipCheck)
        {
            ResetVehicleTransform();
        }
        bPreviousFlipCheck = true;
    }
    else
    {
        bPreviousFlipCheck = false;
    }
}
