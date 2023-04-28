#pragma once

//Akeeper
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Aim_State UMETA(DisplayName = "Aim State"),
	Aim_Walk_State UMETA(DisplayName = "Aim Walk State"),
	Walk_State UMETA(DisplayName = "Walk State"),
	Run_State UMETA(DisplayName = "Run State"),
	Sprint_State UMETA(DisplayName = "Sprint State")
};

USTRUCT(BlueprintType)
struct FCharacterSpeedInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float AimSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float AimWalkSpeed = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkSpeed = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintSpeed = 900.0f;
};

USTRUCT(BlueprintType)
struct FCharacterStaminaInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkConsumption = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float RunConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float SprintConsumption = 5.0f;
};

// UBlueprintFunctionLibrary - класс будет доступен в Blueprints

UCLASS()
class TOPDOWNSHOOTER_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
};

//~Akeeper