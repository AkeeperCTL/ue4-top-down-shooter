// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownShooter/Character/TopDownShooterCharacter.h"

//Akeeper
#include "CoreMinimal.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
//~Akeeper

//#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
//#include "Materials/Material.h"
//#include "Engine/World.h"

ATopDownShooterCharacter::ATopDownShooterCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/Blueprints/Game/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ForwardMoveScale = 0.0f;
	RightMoveScale = 0.0f;
}

void ATopDownShooterCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursorByChannel(TraceTypeQuery6, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATopDownShooterCharacter::SetupPlayerInputComponent(UInputComponent* pNewInputComponent)
{
	Super::SetupPlayerInputComponent(pNewInputComponent);

	if (pNewInputComponent)
	{
		pNewInputComponent->BindAxis(TEXT("MoveForward"), this, &ATopDownShooterCharacter::MoveForwardCallback);
		pNewInputComponent->BindAxis(TEXT("MoveRight"), this, &ATopDownShooterCharacter::MoveRightCallback);
	}
}

void ATopDownShooterCharacter::MoveForwardCallback(float fScale)
{
	ForwardMoveScale = fScale;
}

void ATopDownShooterCharacter::MoveRightCallback(float FScale)
{
	RightMoveScale = FScale;
}

void ATopDownShooterCharacter::MovementTick(float fDeltaSeconds)
{
	UpdateMovementState(fDeltaSeconds);
	
	FVector vForwardDir = FVector(1.0f, 0.0f, 0.0f);
	const FVector vRightDir = FVector(0.0f, 1.0f, 0.0f);
	
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController && PlayerController->IsPlayerController())
	{
		constexpr ETraceTypeQuery FirstAddedTrace = TraceTypeQuery6;
		FHitResult hitResult;

		const bool bSuccess = PlayerController->GetHitResultUnderCursorByChannel(FirstAddedTrace, true, hitResult);
		if (bSuccess)
		{
			const float FYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), hitResult.Location).Yaw;
			SetActorRotation(FRotator(0.0f, FYaw, 0.0f), ETeleportType::None);

			if (MovementState == EMovementState::Sprint_State && ForwardMoveScale > 0 && CurrentStaminaCPP > 0.0f)
				vForwardDir = (hitResult.Location - GetActorLocation()).GetSafeNormal();
		}
	}

	if (MovementState == EMovementState::Sprint_State && ForwardMoveScale > 0 && CurrentStaminaCPP > 0.0f)
		AddMovementInput(vForwardDir, ForwardMoveScale);
	else if (MovementState != EMovementState::Sprint_State)
	{
		AddMovementInput(vForwardDir, ForwardMoveScale);
		AddMovementInput(vRightDir, RightMoveScale);
	}
}

void ATopDownShooterCharacter::UpdateMovementState(float DeltaSeconds)
{
	if (MovementState == EMovementState::Sprint_State)
	{
		if (CurrentStaminaCPP <= 0.0f)
		{
			SprintActivated = false;
			ChangeMovementState();
		}
	}
}

void ATopDownShooterCharacter::UpdateMovementSpeed() const
{
	float ResultSpeed = MovementSpeedInfo.RunSpeed;
	
	switch (MovementState)
	{
	case EMovementState::Aim_State:
			ResultSpeed = MovementSpeedInfo.AimSpeed;
			break;
		case EMovementState::Walk_State:
			ResultSpeed = MovementSpeedInfo.WalkSpeed;
			break;
		case EMovementState::Run_State:
			ResultSpeed = MovementSpeedInfo.RunSpeed;
			break;
		case EMovementState::Sprint_State:
			ResultSpeed = MovementSpeedInfo.SprintSpeed;
			break;
		case EMovementState::Aim_Walk_State:
			ResultSpeed = MovementSpeedInfo.AimWalkSpeed;
			break;
		default:
			break;
	}

	GetCharacterMovement()->MaxWalkSpeed = ResultSpeed;
}

void ATopDownShooterCharacter::ChangeMovementState()
{
	if (!(SprintActivated && ForwardMoveScale > 0 && CurrentStaminaCPP > 0.0f) && !WalkActivated && !AimActivated)
		MovementState = EMovementState::Run_State;
	else
	{
		if (SprintActivated && ForwardMoveScale > 0 && CurrentStaminaCPP > 0.0f)
		{
			MovementState = EMovementState::Sprint_State;
			WalkActivated = false;
			AimActivated = false;
		}

		if (!SprintActivated && WalkActivated && AimActivated)
		{
			MovementState = EMovementState::Aim_Walk_State;
		}
		else
		{
			if (!SprintActivated && !AimActivated && WalkActivated)
			{
				MovementState = EMovementState::Walk_State;
			}
			else
			{
				if (!WalkActivated && !SprintActivated && AimActivated)
				{
					MovementState = EMovementState::Aim_State;
				}
			}
		}
	}
	
	UpdateMovementSpeed();
}

void ATopDownShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// auto Components = GetComponents();
	// for (const auto Component : Components)
	// {
	// 	if (Component->GetName() == "HealthAndStaminaSystem")
	// 	{
	// 		HealthAndStaminaSystem = Component;
	//
	// 		auto Func =  Component->FindFunction("");
	// 	}
	// 	
	// 	UE_LOG(LogTemp, Warning, TEXT("%s"), *(Component->GetName()));
	// }
}
