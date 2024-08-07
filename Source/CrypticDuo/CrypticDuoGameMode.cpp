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

void ACrypticDuoGameMode::HostLANGame()
{
	GetWorld()->ServerTravel("Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}

void ACrypticDuoGameMode::JoinLANGame()
{
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController()) {
		PC->ClientTravel("192.168.7.172", TRAVEL_Absolute);
	}
}
