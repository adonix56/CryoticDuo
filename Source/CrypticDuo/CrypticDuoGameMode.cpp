// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrypticDuoGameMode.h"
#include "CrypticDuoCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACrypticDuoGameMode::ACrypticDuoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
