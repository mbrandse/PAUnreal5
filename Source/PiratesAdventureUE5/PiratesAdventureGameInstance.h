// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RamaSaveStaticSystemData.h"
#include "PiratesAdventureGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPiratesAdventureGameInstance : public UGameInstance
{
	GENERATED_BODY()

public: 

	UPiratesAdventureGameInstance();

	virtual void Init() override;

	//--------------------------------------------------------------------------------------------------------
	// SAVE FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY()
		class URamaSaveStaticSystemData* SystemData;

	UPROPERTY()
		class URamaSaveStaticFileData* CurrentSaveSlotData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save and Load")
		FString SystemSaveName;	

	//Is used to name the slot on the hard drive
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save and Load")
		FString SaveSlotName;

	//The format will be used to create a save format for all files. Don't forget the period. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save and Load")
		FString SaveFormat;

	//We will only allow a fixed number of slots, so our code can be simplified a bit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save and Load")
		int32 MaxSaveSlots;

	uint32 CurrentSaveSlotIndex;

	 bool GetSystemSavePath(FString& FileSavePath);

	//Use the Rama static data save/load for this! Since we can actually load this in without loading the entire world!
	//System data will be used as a separate save file, primarily to record the following things:
	// 1. Filename list (though we may just use the system functions for that) and all associated levels
	// 1b. Basically, make this a struct, with 
	// 1b1. file names
	// 1b2. associated level
	// 1b3. when saved
	// 1b4. index (in list, since we may overwrite some files; we are gonna keep the index to first saved index)
	// 1b5. maybe a screenshot of the point of save
	// 1b6. and perhaps a bunch of other stuff we want to show the player.
	// 2. The last used file (in the main menu we can then set the new "last used" as the current file
	// 3. We can use this for options as well. 
	// 4. We need to know what part of the game we are in: main menu, world map or level. This data may be required for our game mode.
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		bool LoadSystemData(); //TODO: add the parameters we may need.

	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		bool SaveSystemData(); //TODO: add the parameters we may need. 

	//for debugging purposed we want to be able to delete the system data too.
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		bool DeleteSystemData(); 

	//Loading and saving for a singular file
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		class URamaSaveStaticFileData* LoadFileData(UObject* WorldContextObject, int32 FileIndex); //TODO: add the parameters we may need.

	//Saves existing data, either to new slot or old slot
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		bool SaveFileData(UObject* WorldContextObject, int32 FileIndex, FText AreaName, bool bSwitchingMaps, FString StreamingLevel = TEXT(""), FName PlayerStartName = TEXT("")); //TODO: add the parameters we may need.

	//Sets up the next empty save slot, primarily used with new games.
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		int32 GetNextOpenSaveSlot(); 

	//Gets the last saved slot
	UFUNCTION(BlueprintCallable, Category = "Gamemode Functions || Save and Load")
		int32 GetLastSavedSlot(); 

	UFUNCTION(BlueprintPure, Category = "Gamemode Functions || Save and Load")
		int32 SetCurrentSaveSlot(int32 FileIndex, bool bGetLatestSave = false);

	UFUNCTION(BlueprintPure, Category = "Gamemode Functions || Save and Load")
		int32 GetCurrentSaveSlot() { return CurrentSaveSlotIndex; };

	UFUNCTION(BlueprintPure, Category = "Gamemode Functions || Save and Load")
		class URamaSaveStaticFileData* GetCurrentSaveSlotData();

	UFUNCTION(BlueprintPure, Category = "GameMode Functions || Save and Load")
		bool HasSaveFiles();

	//--------------------------------------------------------------------------------------------------------
	// NEW SAVE DATA VALUES TO USE
	//--------------------------------------------------------------------------------------------------------

	/*This is the text that will appear in the main menu.
	Formatting: FileName FileIndex: FileChapter - FileLocation
	However, only for the prologue we will skip file location.*/
	UPROPERTY(EditDefaultsOnly, Category = "New Game Data")
		FText FileHeaderName;

	UPROPERTY(EditDefaultsOnly, Category = "New Game Data")
		FName NewSaveGameMap;

	UPROPERTY(EditDefaultsOnly, Category = "New Game Data")
		FName NewSavePlayerStart;

	//--------------------------------------------------------------------------------------------------------
	// PROGRESS DATA
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Progress Data")
		TMap<EProgressChapter, FText> ChapterNames;

	//--------------------------------------------------------------------------------------------------------
	// LOAD SCREEN FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------
	
	class ARamaSaveEngine* RamaSaveEngine;

	void FullyLoadedGameWorld(FString FileName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Loading Screen", meta = (DisplayName = "On Fully Loaded Game World"))
		void OnReceiveFullyLoadedGameWorld();

	void LevelBlueprintStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Loading Screen", meta = (DisplayName = "On Level Blueprint Started"))
		void OnReceiveLevelBlueprintStarted();
};
