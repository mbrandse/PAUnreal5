// Fill out your copyright notice in the Description page of Project Settings.


#include "PiratesAdventureGameInstance.h"
#include "RamaSaveStaticSystemData.h"
#include "RamaSaveStaticFileData.h"
#include "RamaSaveLibrary.h"
#include "PiratesAdventureUE5.h"
#include "Kismet/BlueprintPathsLibrary.h"
#include "RamaSaveEngine.h"

UPiratesAdventureGameInstance::UPiratesAdventureGameInstance()
{
	SystemSaveName = TEXT("SystemData");
	SaveSlotName = TEXT("Save");
	SaveFormat = TEXT(".sav");
	MaxSaveSlots = 10;
	CurrentSaveSlotIndex = INDEX_NONE;
}

void UPiratesAdventureGameInstance::Init()
{
	if(GetWorld())
	{
		RamaSaveEngine = URamaSaveLibrary::GetOrCreateRamaEngine(GetWorld());
	}

	if (!LoadSystemData())
	{
		//create some messaging here. Our LoadSystemData should properly generate something even if it doesn't exist.
		
	}

	UE_LOG(AnyLog, Log, TEXT("Logging for compile sake"));

	Super::Init();
}

//--------------------------------------------------------------------------------------------------------
// SYSTEM SAVE FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

bool UPiratesAdventureGameInstance::GetSystemSavePath(FString& FileSavePath)
{
	FileSavePath = UBlueprintPathsLibrary::ProjectSavedDir(); //not sure whether this returns the correct path name in packaged builds. Check.
	FileSavePath.Append(SystemSaveName);
	FileSavePath.Append(SaveFormat);

	return UBlueprintPathsLibrary::FileExists(FileSavePath);
}

bool UPiratesAdventureGameInstance::LoadSystemData()
{
	FString FileSavePath;
	if (GetSystemSavePath(FileSavePath))
	{
		bool bSuccess;
		SystemData = Cast<URamaSaveStaticSystemData>(URamaSaveLibrary::RamaSave_LoadStaticDataFromFile(bSuccess, FileSavePath));
		check(SystemData); //do we really need to go this far?

		return SystemData && bSuccess;
	}

	//If we get to this point, we don't have a system save yet.
	SystemData = NewObject<URamaSaveStaticSystemData>(this, URamaSaveStaticSystemData::StaticClass());

	//Make chapter header
	FText ChapterText = FText::FromString(""); //make an empty text
	if (ChapterNames.Contains(EProgressChapter::PC_CHAPTER00))
		ChapterText = ChapterNames.FindRef(EProgressChapter::PC_CHAPTER00);

	//generate empty save slots based on max slots
	for (int32 i = 0; i < MaxSaveSlots; i++)
	{
		FSystemSaveFileData NewSaveSlot;

		FString NewSaveDirectory = UBlueprintPathsLibrary::ProjectSavedDir();
		NewSaveDirectory.Append(SaveSlotName);
		NewSaveDirectory.Append(FString::FromInt(i));
		NewSaveDirectory.Append(SaveFormat);

		NewSaveSlot.bSlotIsEmpty = true; //will be used to ignore UI input
		NewSaveSlot.FileName = FileHeaderName;
		NewSaveSlot.FileChapter = ChapterText;
		NewSaveSlot.FileLocation = FText::FromString("");
		NewSaveSlot.FullSaveDirectory = NewSaveDirectory;
		NewSaveSlot.ActivatedChapters.Add(EProgressChapter::PC_CHAPTER00);

		SystemData->SaveSlots.Add(NewSaveSlot);
	}

	CurrentSaveSlotIndex = 0; //just in case, if an edge case changes our slot index.
	bool bSuccess;
	URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile(FileSavePath, bSuccess, SystemData); //we should have a filesavepath here due to the if earlier. Check if this works.
	return SystemData && bSuccess;
}

bool UPiratesAdventureGameInstance::SaveSystemData()
{
	FString FileSavePath;
	bool bSuccess;

	if(!GetSystemSavePath(FileSavePath)
		|| SystemData == NULL)
		return false; //perhaps have some messaging here.
	
	URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile(FileSavePath, bSuccess, SystemData);

	return SystemData && bSuccess;
}

bool UPiratesAdventureGameInstance::DeleteSystemData()
{
	FString FileSavePath;
	bool bSuccess;

	if (!GetSystemSavePath(FileSavePath))
	{
		bSuccess = URamaSaveLibrary::RamaSave_DeleteFile(FileSavePath);
		if(bSuccess) SystemData = NULL;

		return bSuccess;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------
// FILE SAVE FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

URamaSaveStaticFileData* UPiratesAdventureGameInstance::LoadFileData(UObject* WorldContextObject, int32 FileIndex)
{
	/*we are going to need to make some important design decisions here.
	/ 1. right now it seems we cannot just nilly willy save and load anywhere, even if using Rama GUID. Other level saves just overwrite the data
	/ 2. So until we get clarification from Rama whether this is really how it works, we have two options:
	/ 2a. We create another static save, where we will save arrays of all data that has to carry over between levels.
	/ 2b. We create level based save data, where we need one level save actor that saves to a unique file for that level. 
	/ 
	/ Right now, the first system seems better for one simple reason; we can easily request save data from the required slots, even
	/ if the objects that may eventually request that data are not loaded yet. 
	/ In this case, our save system goes like this:
	/ 1. every single structure we use in arrays need to have a GUID parameter, so we can match those with objects in levels. 
	/ 2. it is better making a structure for every single interactive object that needs saving, including enemies and enemy managers
	/ 3. Instead of using all actors, we need to have the actor request save data from the file, or perhaps we should bind functions to 
	/ the save data (meaning we may need our own delegate). We can likely just load save data, because we are going to do load this on instance level.
	/ 4. Big downside; big array iterations may mean slowdowns if many objects want to get loaded. This needs rigorous testing. 
	/ 5. In which case we may just want to deal with the data from the save data side (which in turns causes problem with level streaming, 
	/ since streaming level objects that are not loaded obviously don't exist on game mode begin play and may load in data already loaded. 
	*/

	if(!SystemData) LoadSystemData(); //make sure all our slots will definitely exist.

	if (SystemData
		&& WorldContextObject
		&& SystemData->SaveSlots.IsValidIndex(FileIndex))
	{
		CurrentSaveSlotIndex = FileIndex;

		//If the file exists, load it. 
		if(URamaSaveLibrary::RamaSave_FileExists(SystemData->SaveSlots[CurrentSaveSlotIndex].FullSaveDirectory))
		{ 			
			bool bSuccess;
			return Cast<URamaSaveStaticFileData>(URamaSaveLibrary::RamaSave_LoadStaticDataFromFile(bSuccess, SystemData->SaveSlots[CurrentSaveSlotIndex].FullSaveDirectory));
		}
		else
		{
			//Otherwise make a new file. 
			URamaSaveStaticFileData* NewFile = NewObject<URamaSaveStaticFileData>(this, URamaSaveStaticFileData::StaticClass());
			NewFile->MapName = NewSaveGameMap;
			NewFile->PlayerStartName = NewSavePlayerStart;
			NewFile->bEnterFromOtherMap = true;
			NewFile->SlotIndex = CurrentSaveSlotIndex;
			//Immediately make a save file so we can use it
			bool bSuccess;
			URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile(SystemData->SaveSlots[CurrentSaveSlotIndex].FullSaveDirectory, bSuccess, NewFile);
			return CurrentSaveSlotData = NewFile;
		}
	}

	return NULL;
}

bool UPiratesAdventureGameInstance::SaveFileData(UObject* WorldContextObject, int32 FileIndex, FText AreaName, bool bSwitchingMaps, FString StreamingLevel/* = TEXT("")*/, FName PlayerStartName/* = TEXT("")*/)
{
	if (!SystemData) LoadSystemData();

	if (SystemData
		&& WorldContextObject)
	{
		if(SystemData->SaveSlots.IsValidIndex(FileIndex))
		{
			if (FileIndex != CurrentSaveSlotIndex)
			{
				//If our current active data and FileIndex don't match, the user is saving to a different slot.
				SystemData->SaveSlots[FileIndex].FileName =				SystemData->SaveSlots[CurrentSaveSlotIndex].FileName;
				SystemData->SaveSlots[FileIndex].FileChapter =			SystemData->SaveSlots[CurrentSaveSlotIndex].FileChapter;
				SystemData->SaveSlots[FileIndex].FileLocation =			AreaName;
			}

			//Set index as our new primary, just in case we overwrote the file before
			CurrentSaveSlotIndex = FileIndex;

			SystemData->SaveSlots[CurrentSaveSlotIndex].bSlotIsEmpty = false; //set it to false; since that signifies this slot has been saved to at least once.
			SystemData->SaveSlots[CurrentSaveSlotIndex].SaveTime = FDateTime::Now();
			
			SaveSystemData();

			//REMAKE THIS PART!!!
			//include checks to make sure the save file index is matched to the new index in case the 

			//bool bSuccess, bSavedAllComps;
			//URamaSaveLibrary::RamaSave_SaveToFile(WorldContextObject, SystemData->SaveSlots[CurrentSaveSlotIndex].FullSaveDirectory, bSuccess, bSavedAllComps);
			//return bSuccess && bSavedAllComps;

			//else we got a streaming map do the other load
			return true;
		}
	}

	//if everything fails, return false
	return false;
}

int32 UPiratesAdventureGameInstance::GetNextOpenSaveSlot()
{
	if (SystemData)
	{
		for (int32 i = 0; i < SystemData->SaveSlots.Num(); i++)
		{
			if(SystemData->SaveSlots[i].bSlotIsEmpty)
				return i;
		}
	}

	return 0; //if all slots are full, just reset to slot 0
}

int32 UPiratesAdventureGameInstance::GetLastSavedSlot()
{
	//FDateTime CurrentHighestTime = FDateTime(0,0,0,0,0,0,0);
	FDateTime CurrentHighestTime;
	int32 CurrentIndex = INDEX_NONE;

	if (SystemData)
	{
		for (int32 i = 0; i < SystemData->SaveSlots.Num(); i++)
		{
			if (!SystemData->SaveSlots[i].bSlotIsEmpty)
			{
				if (SystemData->SaveSlots[i].SaveTime > CurrentHighestTime
					|| i == 0)
				{
					//since our only options of initializing DateTime will directly screw over the "last save" search, 
					//let's use the index == 0 to set the first DateTime instead. 
					CurrentHighestTime = SystemData->SaveSlots[i].SaveTime;
					CurrentIndex = i;
				}
			}
		}
	}

	return CurrentIndex; //will return INDEX_NONE if we have no slots or all slots have unsaved SaveTime
}

int32 UPiratesAdventureGameInstance::SetCurrentSaveSlot(int32 FileIndex, bool bGetLatestSave /* = false */)
{
	if(FileIndex != CurrentSaveSlotIndex
		&& GetWorld())
	{
		int32 NewFileIndex = bGetLatestSave ? GetLastSavedSlot() : FileIndex;
		CurrentSaveSlotData = LoadFileData(GetWorld(), NewFileIndex);
		return CurrentSaveSlotIndex = NewFileIndex;
	}

	return CurrentSaveSlotIndex;
}

URamaSaveStaticFileData* UPiratesAdventureGameInstance::GetCurrentSaveSlotData()
{
	if (!CurrentSaveSlotData
		&& GetWorld()
		&& CurrentSaveSlotIndex != INDEX_NONE
		&& SystemData
		&& SystemData->SaveSlots.IsValidIndex(CurrentSaveSlotIndex))
	{
		CurrentSaveSlotData = LoadFileData(GetWorld(), CurrentSaveSlotIndex);
	}

	return CurrentSaveSlotData;
}

bool UPiratesAdventureGameInstance::HasSaveFiles()
{
	if(SystemData)
		return SystemData->SaveSlots.Num() != 0;

	return false;
}

//--------------------------------------------------------------------------------------------------------
// LOADING SCREEN FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

void UPiratesAdventureGameInstance::FullyLoadedGameWorld(FString FileName)
{
	
}

void UPiratesAdventureGameInstance::LevelBlueprintStarted()
{
	OnReceiveLevelBlueprintStarted();
}