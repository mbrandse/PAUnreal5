// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDElementBase.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPlayerHUDElementBase : public UUserWidget
{
	GENERATED_BODY()

public:

	//--------------------------------------------------------------------------------------------------------
	//  OWNERSHIP
	//--------------------------------------------------------------------------------------------------------

	class APlayerHUD* ElementOwner;

	UFUNCTION(BlueprintPure, Category = "HUD")
		class APlayerHUD* GetElementOwner() { return ElementOwner; };
	
};
