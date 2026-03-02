// Copyright Simon Kramer. All Rights Reserved.


#include "AI/TurboAIVehicle.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ATurboAIVehicle::ATurboAIVehicle()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
    SpringArm->TargetArmLength = 6000.0f;
    SpringArm->bDoCollisionTest = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->SetAutoActivate(false);
}

void ATurboAIVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

}

void ATurboAIVehicle::BeginPlay()
{
    Super::BeginPlay();

    if (bEnableSpectatorCamera && Camera)
    {
        Camera->Activate();
    }
}

