// Fill out your copyright notice in the Description page of Project Settings.


#include "PiratesAdventureScriptActor.h"
#include "PiratesAdventureGameInstance.h"
#include "Kismet/GameplayStatics.h"

void APiratesAdventureScriptActor::BeginPlay()
{
	if (GetWorld())
	{
		UPiratesAdventureGameInstance* GameInstance = Cast<UPiratesAdventureGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if(GameInstance)
			GameInstance->LevelBlueprintStarted();
	}

	Super::BeginPlay();
}
