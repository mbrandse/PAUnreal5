// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "PiratesAdventurePlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API APiratesAdventurePlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	
	APiratesAdventurePlayerStart(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pirates Adventure")
		FName PlayerStartName;
	
};
