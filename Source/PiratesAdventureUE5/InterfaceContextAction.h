// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InterfaceContextAction.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInterfaceContextAction : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PIRATESADVENTUREUE5_API IInterfaceContextAction
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, Category = "Context")
		void StartContextAction(class APlayerCharacter* OverlappingPlayer);

	UFUNCTION(BlueprintNativeEvent, Category = "Context")
		void StopContextAction(class APlayerCharacter* OverlappingPlayer);
};
