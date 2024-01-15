// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoUE5FSMGameMode.h"
#include "DemoUE5FSMCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADemoUE5FSMGameMode::ADemoUE5FSMGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
