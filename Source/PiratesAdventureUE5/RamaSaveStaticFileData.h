// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RamaSaveObject.h"
#include "PiratesAdventureStaticLibrary.h"
#include "RamaSaveStaticFileData.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API URamaSaveStaticFileData : public URamaSaveObject
{
	GENERATED_BODY()

public:
	
	//--------------------------------------------------------------------------------------------------------
	// SLOT RELATED DATA
	//--------------------------------------------------------------------------------------------------------

	int32 SlotIndex;

	//--------------------------------------------------------------------------------------------------------
	// SAVE DATA PLAYER LOCATION
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bEnterFromOtherMap = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FName MapName = TEXT("");

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FName StreamingLevel = TEXT(""); //In case we saved in the middle of a map.

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FName PlayerStartName = TEXT(""); //This is the playerstart ID; not to be confused with the FileLocation (which is purely aesthetics)

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		FVector SavedPlayerLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		EPlayerMovementDirection SavedPlayerDirection;

	//--------------------------------------------------------------------------------------------------------
	// SAVE DATA PLAYER
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		float SavedMaxHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		float SavedCurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		int32 SavedCoins;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		TArray<FSaveDataPlayerElement> SavedElementData;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		FSaveDataPlayerElement SavedEquippedElementData;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		TArray<FSaveDataPlayerTool> SavedToolData;

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		FSaveDataPlayerTool SavedEquippedToolData;

	//Have a variable for unlocked costumes

	//Have a variable for items/consumables

	UPROPERTY(BlueprintReadOnly, Category = "Player Data")
		int32 SavedEnemyKillCount;

	//Have a variable for completed conversations

	//--------------------------------------------------------------------------------------------------------
	// SAVE DATA ENVIRONMENT
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Environment Data")
		TArray<FSaveDataOverworld> SavedOverworldData;
	
	UPROPERTY(BlueprintReadOnly, Category = "Environment Data")
		TArray<FSaveDataSwitch> SavedSwitchData;

	UPROPERTY(BlueprintReadOnly, Category = "Environment Data")
		TArray<FSaveDataDoor> SavedDoorData;

	UPROPERTY(BlueprintReadOnly, Category = "Environment Data")
		TArray<FSaveDataLadder> SavedLadderData;

	UPROPERTY(BlueprintReadOnly, Category = "Environment Data")
		TArray<FSaveDataElevator> SavedElevatorData;
};
