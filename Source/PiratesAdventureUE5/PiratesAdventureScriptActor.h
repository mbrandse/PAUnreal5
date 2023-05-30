// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "PiratesAdventureScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API APiratesAdventureScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	
};
