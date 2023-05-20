// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownShooter/Character/TopDownShooterCharacter.h"

//Akeeper
#include "CoreMinimal.h"
#include "Engine/Classes/Kismet/KismetMathLibrary.h"
//~Akeeper

#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


ATopDownShooterCharacter::ATopDownShooterCharacter()
{
	// Set size for player capsule
	this->GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	this->bUseControllerRotationPitch = false;
	this->bUseControllerRotationYaw = false;
	this->bUseControllerRotationRoll = false;

	// Configure character movement
	this->GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	this->GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	this->GetCharacterMovement()->bConstrainToPlane = true;
	this->GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	this->CameraBoom->SetupAttachment(RootComponent);
	this->CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	this->CameraBoom->TargetArmLength = 800.f;
	this->CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	this->CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	this->TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	this->TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	this->TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	this->CursorDecalMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/Blueprints/Game/M_Cursor_Decal.M_Cursor_Decal'")).Object;
	
	// Activate ticking in order to update the cursor every frame.
	this->PrimaryActorTick.bCanEverTick = true;
	this->PrimaryActorTick.bStartWithTickEnabled = true;

	this->MoveScale2D = FVector(0,0,0);
}

void ATopDownShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
		this->CursorDecal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorDecalMaterial, CursorDecalSize, FVector(0));
}

void ATopDownShooterCharacter::CursorDecalTick() const
{
	if (!this->CursorDecal)
		UE_LOG(LogTemp, Error, TEXT("Error in file %s and line %d"), __FILE__, __LINE__);

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		UE_LOG(LogTemp, Error, TEXT("Error in file %s and line %d"), __FILE__, __LINE__);
	
	constexpr float TraceDistance = 10000.0f;
	FVector WorldLocation;
	FVector WorldDirection;
	
	PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
	Params.AddIgnoredActor(this);
	
	FHitResult TraceHitResult;
	//PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		WorldLocation,
		WorldLocation + WorldDirection * TraceDistance,
		ECC_Visibility,
		Params);
	
	const FQuat SurfaceRotation = TraceHitResult.ImpactNormal.ToOrientationQuat();
	this->CursorDecal->SetWorldLocationAndRotation(TraceHitResult.Location, SurfaceRotation);
}

void ATopDownShooterCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
	
	MovementTick(DeltaSeconds);
	CursorDecalTick();
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

void ATopDownShooterCharacter::MoveForwardCallback(const float Scale)
{
	this->MoveScale2D.X = Scale;
}

void ATopDownShooterCharacter::MoveRightCallback(const float Scale)
{
	this->MoveScale2D.Y = Scale;
}

void ATopDownShooterCharacter::MovementTick(const float DeltaSeconds)
{
	UpdateMovementState(DeltaSeconds);
	
	FVector ForwardDir = FVector(1.0f, 0.0f, 0.0f);
	const FVector RightDir = FVector(0.0f, 1.0f, 0.0f);

	//const FVector BackDir = FVector(-1.0f, 0.0f, 0.0f);
	//const FVector LeftDir = FVector(0.0f, -1.0f, 0.0f);

	const FVector actorLocation = GetActorLocation();
	const FVector actorLookAtSprintLoc = actorLocation + MoveScale2D;
	
	const APlayerController* PlayerController = Cast<APlayerController>(this->GetController());
	if (PlayerController && PlayerController->IsPlayerController())
	{
		float Yaw = UKismetMathLibrary::FindLookAtRotation(actorLocation, actorLookAtSprintLoc).Yaw;
		
		FHitResult HitResult;
		constexpr auto CursorLandscapeChannel = ECC_GameTraceChannel1;

		//Поворот персонажа в спринте теперь соответствует направлению движения
		const bool bSuccess = PlayerController->GetHitResultUnderCursor(CursorLandscapeChannel, true, HitResult);
		if (bSuccess && MovementState != EMovementState::Sprint_State)
		{
			Yaw = UKismetMathLibrary::FindLookAtRotation(actorLocation, HitResult.Location).Yaw;
			
			//if (MovementState == EMovementState::Sprint_State && MoveScale2D.X > 0 && CurrentStaminaCPP > 0.0f)
				//ForwardDir = (HitResult.Location - GetActorLocation()).GetSafeNormal();
		}

		this->SetActorRotation(FRotator(0.0f, Yaw, 0.0f), ETeleportType::None);
	}

	//if (MovementState == EMovementState::Sprint_State && MoveScale2D.X > 0 && CurrentStaminaCPP > 0.0f)
		//AddMovementInput(ForwardDir, MoveScale2D.X);
	//if (MovementState != EMovementState::Sprint_State)
	//{
		AddMovementInput(ForwardDir, MoveScale2D.X);
		AddMovementInput(RightDir, MoveScale2D.Y);
	//}
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
	if (!(SprintActivated && CurrentStaminaCPP > 0.0f) && !WalkActivated && !AimActivated)
		MovementState = EMovementState::Run_State;
	else
	{
		// if (SprintActivated && MoveScale2D.X > 0 && CurrentStaminaCPP > 0.0f)
		if (SprintActivated && CurrentStaminaCPP > 0.0f)
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
