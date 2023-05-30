// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RamaSaveObject.h"
#include "RamaSaveStaticSystemData.generated.h"

UENUM(BlueprintType)
enum class EProgressChapter : uint8
{
	PC_NONE UMETA(DisplayName = "NULL"), //default value, not used. 
	PC_CHAPTER00 UMETA(DisplayName = "Prologue"),
	PC_CHAPTER01 UMETA(DisplayName = "Chapter 01"),
	PC_CHAPTER02 UMETA(DisplayName = "Chapter 02")
};

USTRUCT(BlueprintType) //we likely don't need this to be blueprint type as we can only read it there anyway.
struct FSystemSaveFileData
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bSlotIsEmpty = true; //this will determine whether the slot is empty (i.e. no data can be loaded from it)
	
	//UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	//	int32 FileIndex = INDEX_NONE; //Do we really need this? Isn't the array index more than enough?

	//This is how we name the slot: FileName FileIndex: FileChapter - FileLocation
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FText FileName = FText::FromString(TEXT(""));

	//This is how we name the slot: FileName FileIndex: FileChapter - FileLocation
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FText FileChapter = FText::FromString(TEXT(""));

	/*This is how we name the slot : FileName FileIndex : FileChapter - FileLocation
	* This is NOT the save file location. Use FullSaveDirectory for that */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FText FileLocation = FText::FromString(TEXT(""));

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FString FullSaveDirectory = TEXT("");

	//BAHAHA! actually setting this as zero value crashes the editor at 75%. So find a different way of initializing this. 
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FDateTime SaveTime/* = FDateTime(0, 0, 0, 0, 0, 0, 0)*/; //zeroed out just in case, so we don't get any "not initialized" complaints.

	//We may have non-linear chapter progression, so this array can be used to make sure we havent accessed a chapter yet (so we don't get
	//superfluous "CHAPTER 1 THIS IS A CHAPTER" titlecards
	TArray<EProgressChapter> ActivatedChapters;

	//TODO: add player status data as well; max health, unlocked elements and tools. Necessary for UI visualization
};

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API URamaSaveStaticSystemData : public URamaSaveObject
{
	GENERATED_BODY()

public:

	URamaSaveStaticSystemData();

	//System save data
	TArray< FSystemSaveFileData> SaveSlots;
	int32 LastUsedSaveDataIndex;
	//TODO: as an autosave option (single one for now is fine)
	
	//Setup the options here too.
};
