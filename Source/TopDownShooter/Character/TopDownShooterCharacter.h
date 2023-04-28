// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TopDownShooter/Types.h"
#include "TopDownShooterCharacter.generated.h"

UCLASS(Blueprintable)
class ATopDownShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATopDownShooterCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() const { return CursorToWorld; }

private:

	//class UActorComponent* HealthAndStaminaSystem;;
	
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;
	
public:
	//Properties
	//Akeeper
	float ForwardMoveScale;
	float RightMoveScale;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Movement")
		float CurrentStaminaCPP = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		bool AimActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		bool WalkActivated = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		bool SprintActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		EMovementState MovementState = EMovementState::Run_State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		FCharacterSpeedInfo MovementSpeedInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
		FCharacterStaminaInfo MovementStaminaConsumption;
	//~Properties

	//Methods
	virtual void SetupPlayerInputComponent(UInputComponent* NewInputComponent) override;
	virtual void PostInitializeComponents() override;

	//Movement representation
	void MoveForwardCallback(float Scale);
	void MoveRightCallback(float Scale);
	void MovementTick(float DeltaSeconds);
	void UpdateMovementState(float DeltaSeconds);

	//Изменение скорости персонажа в зависимости
	//от состояния движения (бег, ходьба, прицеливание)
	UFUNCTION(BlueprintCallable)
	void UpdateMovementSpeed() const;

	//Изменение состояния движения (бег, ходьба, прицеливание)
	UFUNCTION(BlueprintCallable)
	void ChangeMovementState();
	//~Akeeper
	//~Methods
};

