// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrypticDuoGameMode.generated.h"

UCLASS(minimalapi)
class ACrypticDuoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACrypticDuoGameMode();

	UFUNCTION(BlueprintCallable)
	void HostLANGame();

	UFUNCTION(BlueprintCallable)
	void JoinLANGame();
};



