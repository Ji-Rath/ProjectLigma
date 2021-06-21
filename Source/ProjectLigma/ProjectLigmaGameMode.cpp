// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectLigmaGameMode.h"
#include "ProjectLigmaPlayerController.h"
#include "ProjectLigmaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectLigmaGameMode::AProjectLigmaGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AProjectLigmaPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}