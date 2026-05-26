// Copyright Epic Games, Inc. All Rights Reserved.

#include "papureGameMode.h"
#include "papurePlayerController.h"
#include "papureCharacter.h"
#include "UObject/ConstructorHelpers.h"

ApapureGameMode::ApapureGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ApapurePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}