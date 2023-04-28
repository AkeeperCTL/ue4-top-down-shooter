// Copyright Epic Games, Inc. All Rights Reserved.

#include "TopDownShooterGameMode.h"
#include "TopDownShooter/Character/TopDownShooterCharacter.h"
#include "TopDownShooter/Character/TopDownShooterPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATopDownShooterGameMode::ATopDownShooterGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATopDownShooterPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character 
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/Human/BP_Player"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}