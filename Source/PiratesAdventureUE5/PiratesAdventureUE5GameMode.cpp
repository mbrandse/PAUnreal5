// Copyright Epic Games, Inc. All Rights Reserved.

#include "PiratesAdventureUE5GameMode.h"
#include "PiratesAdventureUE5Character.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PiratesAdventurePlayerStart.h"
#include "PlayerCharacter.h"
#include "PiratesAdventureGameInstance.h"

APiratesAdventureUE5GameMode::APiratesAdventureUE5GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/UE5StarterContent/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

AActor* APiratesAdventureUE5GameMode::ChoosePlayerStart(AController* Player)
{
	if (GetWorld())
	{
		//First get the player character and load it
		//This will be our current player. At this point we should assume we already locked in on a particular save file.
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Player->GetPawn());
		if(PlayerCharacter)
		{
			UPiratesAdventureGameInstance* GameInstance = Cast<UPiratesAdventureGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
			check(GameInstance); //if we don't have the game instance, we are basically screwed. 
			if(GameInstance->GetCurrentSaveSlotData())
			{
				TArray<AActor*> PlayerStarts;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts); //get the default player starts here first, since we have to cast later anyway.
				if(PlayerStarts.Num() == 0) return NULL;

				if (GameInstance->GetCurrentSaveSlotData()->PlayerStartName != TEXT("")
					&& GameInstance->GetCurrentSaveSlotData()->bEnterFromOtherMap)
				{
					FName PlayerStartName = GameInstance->GetCurrentSaveSlotData()->PlayerStartName;
					for (AActor* PlayerStart : PlayerStarts)
					{
						APiratesAdventurePlayerStart* NewPlayerStart = Cast<APiratesAdventurePlayerStart>(PlayerStart);
						if (!NewPlayerStart) continue;

						if (NewPlayerStart->PlayerStartName == PlayerStartName)
							return NewPlayerStart;
					}
				}

				return PlayerStarts[0];	
			}
		}
	}

	//if no playercharacter or world, just rely on default implementation
	return Super::ChoosePlayerStart(Player);
}
