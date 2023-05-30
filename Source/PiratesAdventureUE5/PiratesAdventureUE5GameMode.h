// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PiratesAdventureUE5GameMode.generated.h"

UCLASS(minimalapi)
class APiratesAdventureUE5GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APiratesAdventureUE5GameMode();

	AActor* ChoosePlayerStart(AController* Player); 
};



