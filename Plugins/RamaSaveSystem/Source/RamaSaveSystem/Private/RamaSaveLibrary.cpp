// Copyright 2018 by Nathan "Rama" Iyer. All Rights Reserved.
#include "RamaSaveSystemPrivatePCH.h"
#include "RamaSaveLibrary.h"
 
#include "RamaSaveSystemSettings.h"

 
//////////////////////////////////////////////////////////////////////////
// URamaSaveLibrary



template <class FunctorType>
class PlatformFileFunctor : public IPlatformFile::FDirectoryVisitor	//GenericPlatformFile.h
{
public:
	
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		return Functor(FilenameOrDirectory, bIsDirectory);
	}

	PlatformFileFunctor(FunctorType&& FunctorInstance)
		: Functor(MoveTemp(FunctorInstance))
	{
	}

private:
	FunctorType Functor;
};
 
template <class Functor>
PlatformFileFunctor<Functor> MakeDirectoryVisitor(Functor&& FunctorInstance)
{
	return PlatformFileFunctor<Functor>(MoveTemp(FunctorInstance));
}

//This is my own algorithm from scratch
// If you use this elsewhere please credit me somewhere appropriate
//  - Rama
bool URamaSaveLibrary::RamaFileIO_GetFiles(const FString& FullPathOfBaseDir, TArray<FString>& FilenamesOut, bool Recursive, const FString& FilterByExtension)
{
	//Format File Extension, remove the "." if present
	const FString FileExt = FilterByExtension.Replace(TEXT("."),TEXT("")).ToLower();
	
	FString Str;
	auto FilenamesVisitor = MakeDirectoryVisitor(
		[&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) 
		{
			//Files
			if ( ! bIsDirectory)
			{
				//Filter by Extension
				if(FileExt != "")
				{
					Str = FPaths::GetCleanFilename(FilenameOrDirectory); //removes the dot, unless use true as param
				
					FString LowerExt = FPaths::GetExtension(Str).ToLower();
					
					bool Match = (LowerExt == FileExt);
								
					//Filter by Extension
					if(Match)  
					{
						if(Recursive) 
						{
							FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
						}
						else 
						{
							FilenamesOut.Push(Str);
						}
					}
				}
				
				//Include All Filenames!
				else
				{
					//Just the Directory
					Str = FPaths::GetCleanFilename(FilenameOrDirectory);
					
					if(Recursive) 
					{
						FilenamesOut.Push(FilenameOrDirectory); //need whole path for recursive
					}
					else 
					{
						FilenamesOut.Push(Str);
					}
				}
			}
			return true;
		}
	);
	if(Recursive) 
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*FullPathOfBaseDir, FilenamesVisitor);
	}
	else 
	{
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*FullPathOfBaseDir, FilenamesVisitor);
	}
}	
	
//Verify all UObject * properties are actual game assets and not runtime created UObjects which cannot be loaded from disk directly.
// Ex: Material Instance Dynamics vs Material Instance Constants which exist as Content folder assets
bool URamaSaveLibrary::VerifyActorAndComponentProperties(URamaSaveComponent* SaveComp)
{
	if(!SaveComp) 
	{
		return false;
	}
	
	AActor* ActorOwner = SaveComp->GetOwner();
	if(!ActorOwner) 
	{
		return false;
	}
	
	//Actor
	for (TFieldIterator<FProperty> It(ActorOwner->GetClass()); It; ++It)
	{
		FProperty* Property = *It;
		FString PropertyNameString = Property->GetFName().ToString();
		
		//Is this property in the list of properties to save to disk?
		if(!SaveComp->RamaSave_OwningActorVarsToSave.Contains(PropertyNameString))
		{ 
			//Skip
			continue;
		}
		 
		//Check Object Properties
		//UObjectProperty* ObjProp = Cast<UObjectProperty>(Property);
		FObjectProperty* ObjProp = CastField<FObjectProperty>(Property);

		if(ObjProp)
		{    
			UObject* Obj = ObjProp->GetObjectPropertyValue_InContainer(ActorOwner);
			if(!Obj)
			{
				continue;
			}

			//Verfiy this is a load-able UObject Ptr !  <3 Rama
			if(!URamaSaveUtility::VerifyObjectCanBeLoaded(Obj))
			{ 	
				VSCREENMSGSEC(12, "Save Process has been cancelled!");
				VSCREENMSGSEC(12, "Please remove the invalid properties from the save name array, RamaSave_OwningActorVarsToSave"); 
				
				FString Msg = "The variable ~ " + PropertyNameString + " ~ found in " + ActorOwner->GetClass()->GetName();
				Msg += " >> This type of UObject Ptr cannot be saved/loaded directly. Save individual variable values and recreate in Actor Fully Loaded Event <3 Rama";
				VSCREENMSGSEC(12, Msg);
				UE_LOG(RamaSave,Error,TEXT("%s"), *Msg);
				 
				VSCREENMSGSEC(12, "~~~ Rama Save System Message ~~~");
				 
				return false;
			}  
		} 
	}
	
	//Save Component
	for (TFieldIterator<FProperty> It(SaveComp->GetClass()); It; ++It)
	{
		FProperty* Property = *It;
		FString PropertyNameString = Property->GetFName().ToString();
		
		//Check Object Properties
		FObjectProperty* ObjProp = CastField<FObjectProperty>(Property);
		if(ObjProp)
		{    
			UObject* Obj = ObjProp->GetObjectPropertyValue_InContainer(SaveComp);
			if(!Obj)
			{
				continue;
			}
 
			//Verfiy this is a load-able UObject Ptr !  <3 Rama
			if(!URamaSaveUtility::VerifyObjectCanBeLoaded(Obj))
			{ 	
				VSCREENMSGSEC(12, "Save Process has been cancelled!");
				VSCREENMSGSEC(12, "Please remove the invalid properties from the Rama Save Component");
				 
				FString Msg = "The variable ~ " + PropertyNameString + " ~ found in " + SaveComp->GetClass()->GetName();
				Msg += " >> This type of UObject Ptr cannot be saved/loaded directly. Save individual variable values and recreate in Actor Fully Loaded Event <3 Rama";
				VSCREENMSGSEC(12, Msg);
				UE_LOG(RamaSave,Error,TEXT("%s"), *Msg);
				  
				VSCREENMSGSEC(12, "~~~ Rama Save System Message ~~~");
				
				return false;
			}  
		} 
	}
	 
	return true;
}

 
FString URamaSaveLibrary::RemoveLevelPIEPrefix(const FString& LevelName)
{ 
	FString Left,Result;
	
	//Remove Game/Maps/... etc
	LevelName.Split(TEXT("/"),&Left,&Result, ESearchCase::IgnoreCase, ESearchDir::FromEnd); //<~~~
	  
	//UEDPIE_x_ActualLevelName
	// The hope is not never having x be double digits (system will break if that happens, its usually 0)
	if(Result.Contains(TEXT("UEDPIE")))
	{
		return Result.RightChop(9); //UEDPIE_x_
	} 
	return Result;
}

bool URamaSaveLibrary::RamaSave_CancelAsyncSaveProcess(UObject* WorldContextObject)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return false;
	
	/*
	if(!World->IsServer())
	{
		VSCREENMSG("Rama Save System ~ Saving can only be done by the Server!");
		return false;
	}
	*/
	
	//Non replicated actor
	ARamaSaveEngine* RamaEngine = URamaSaveLibrary::GetOrCreateRamaEngine(World);
	if(!RamaEngine)
	{
		VSCREENMSG("Rama Save System ~ Save Engine Actor could not created, tell Rama!");
		return false;
	}
	
	return RamaEngine->RamaSaveAsync_Cancel();
}


void URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile(
	FString FileName, 
	bool& FileIOSuccess, 
	URamaSaveObject* StaticSaveData
){
	if(!StaticSaveData)
	{
		VSCREENMSG2("URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile ~ No Static Data Object supplied while saving to location!", FileName);
		return;
	}
	
	FileIOSuccess = false;
	if(!URamaSaveUtility::CreateDirectoryTreeForFile(FileName))
	{
		VSCREENMSG2("URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile ~ Directory could not be created!", FileName);
		RS_LOG2(RamaSave,"URamaSaveLibrary::RamaSave_SaveOnlyStaticToFile ~ Directory could not be created!", FileName);
		return;
	}
	
	ARamaSaveEngine::RamaSave_SaveStaticData(FileName,FileIOSuccess,StaticSaveData);
}
	
void URamaSaveLibrary::RamaSave_SaveToFile_WithTags(
	UObject* WorldContextObject, 
	FString FileName, bool& FileIOSuccess, bool& AllComponentsSaved,
	TArray<FString> SaveOnlyActorsWithTags,
	FString SaveOnlyStreamingLevel,
	bool IgnoreStreamingActorsIfNoLevelSpecified,
	URamaSaveObject* StaticSaveData)
{
	if (!WorldContextObject) return;

	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return;
	
	/*
	if(!World->IsServer())
	{
		VSCREENMSG("Rama Save System ~ Saving can only be done by the Server!");
		return;
	}
	*/
	
	ARamaSaveEngine* RamaEngine = URamaSaveLibrary::GetOrCreateRamaEngine(World);
	if(!RamaEngine)
	{
		VSCREENMSG("Rama Save System ~ Save Engine Actor could not created, tell Rama!");
		return;
	}
	
	RamaEngine->RamaSave_SaveToFile(FileName,FileIOSuccess,AllComponentsSaved,SaveOnlyActorsWithTags,SaveOnlyStreamingLevel,IgnoreStreamingActorsIfNoLevelSpecified,StaticSaveData);
}
	 

ARamaSaveEngine* URamaSaveLibrary::GetOrCreateRamaEngine(UWorld* World)
{
	if(!World) return nullptr;
	
	//Try to find existing
	TActorIterator<ARamaSaveEngine> Itr(World); 
	if(Itr)
	{
		return *Itr;
	}
	
	//Create!
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	SpawnInfo.Owner 				= World->GetGameState<AGameState>();
	SpawnInfo.bDeferConstruction 	= false;
	  
	URamaSaveSystemSettings* Settings = URamaSaveSystemSettings::Get();
	
	//Should always be valid!
	check(Settings);
	
	TSubclassOf<ARamaSaveEngine> RamaEngineClass = Settings->RamaSaveEngineBP;
	if(!RamaEngineClass) 
	{
		RS_LOG(RamaSave,"Rama Save Engine class in RamaSaveSystemSettings was invalid!!!");
		RamaEngineClass = ARamaSaveEngine::StaticClass();
	}
	 
	return World->SpawnActor<ARamaSaveEngine>(RamaEngineClass, FVector(0.333,0.333,10240), FRotator::ZeroRotator, SpawnInfo );
}

void URamaSaveLibrary::RamaSave_LoadFromFile(UObject* WorldContextObject, bool& FileIOSuccess, FString FileName, bool DestroyActorsBeforeLoad, bool DontLoadPlayerPawns, bool HandleStreamingLevelsLoadingAndUnloading, FString LoadOnlyStreamingLevel)
{
	//Goal is to simplify the BP note for newer users so they dont have to consider the tags input variable at all until/unless they are ready to :)
	TArray<FString> LoadOnlyActorsWithSaveTags;
	 
	RamaSave_LoadFromFileWithTags(WorldContextObject,LoadOnlyActorsWithSaveTags,FileIOSuccess,FileName,DestroyActorsBeforeLoad,DontLoadPlayerPawns,HandleStreamingLevelsLoadingAndUnloading,LoadOnlyStreamingLevel);
}

URamaSaveObject* URamaSaveLibrary::RamaSave_LoadStaticDataFromFile(bool& FileIOSuccess,  FString FileName)
{
	return ARamaSaveEngine::LoadStaticData(FileIOSuccess,FileName);
}

void URamaSaveLibrary::RamaSave_LoadFromFileWithTags(UObject* WorldContextObject, const TArray<FString>& LoadOnlyActorsWithSaveTags, bool& FileIOSuccess, FString FileName, bool DestroyActorsBeforeLoad, bool DontLoadPlayerPawns, bool HandleStreamingLevelsLoadingAndUnloading, FString LoadOnlyStreamingLevel)
{
	FileIOSuccess = false;
	
	
	if(!URamaSaveUtility::FileExists(FileName))
	{
		VSCREENMSG2("Rama Save System ~ File not found!", FileName);
		return;
	}
	 
	if(!WorldContextObject) return;
	 
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return;
	
	bool IsServer = World->GetNetMode() == ENetMode::NM_Standalone || World->GetNetMode() == ENetMode::NM_DedicatedServer || World->GetNetMode() == ENetMode::NM_ListenServer;
	if(!IsServer)
	{
		VSCREENMSG("Rama Save System ~ Loading can only be done by the Server!");
		return; 
	}
	
	//Victory Decompress File
	TArray<uint8> Uncompressed_FromBinary;
	if( !URamaSaveUtility::DecompressFromFile(FileName,Uncompressed_FromBinary))
	{
		//File could not be loaded!
		VSCREENMSG("Rama Save System ~ File was found but could not be loaded! " + FileName );
		return;
	}
	
	FileIOSuccess = true;
	
	//~~~
	
	ARamaSaveEngine* RamaEngine = URamaSaveLibrary::GetOrCreateRamaEngine(World);
	if(!RamaEngine)
	{
		VSCREENMSG("Rama Save Engine Actor could not be created! Tell Rama!");
		RS_LOG(RamaSave,"Rama Save Engine Actor could not be created! Tell Rama!");
		return;
	}
	
	FRamaSaveEngineParams Params;
	Params.LoadOnlyActorsWithSaveTags 	= LoadOnlyActorsWithSaveTags; 
	Params.FileName 					= FileName; 
	Params.DestroyActorsBeforeLoad 		= DestroyActorsBeforeLoad; 
	Params.DontLoadPlayerPawns 			= DontLoadPlayerPawns;
	Params.LoadOnlyStreamingLevel 		= LoadOnlyStreamingLevel;
	
	//Launch Async Load Process!
	RamaEngine->Phase1(Params,HandleStreamingLevelsLoadingAndUnloading);
}
 
int32 URamaSaveLibrary::RamaSave_LoadStreamingStateFromFile(UObject* WorldContextObject, bool& FileIOSuccess, FString FileName, TArray<FString>& StreamingLevelsStates)
{
	FileIOSuccess = false;
	
	if(!URamaSaveUtility::FileExists(FileName))
	{
		VSCREENMSG2("Rama Save System ~ File not found!", FileName);
		return 0;
	}
	
	 
	if(!WorldContextObject) return 0;
	 
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return 0;
	
	bool IsServer = World->GetNetMode() == ENetMode::NM_Standalone || World->GetNetMode() == ENetMode::NM_DedicatedServer || World->GetNetMode() == ENetMode::NM_ListenServer;
	if(!IsServer)
	{
		VSCREENMSG("Rama Save System ~ Loading can only be done by the Server!");
		return 0; 
	}
	
	//Victory Decompress File
	TArray<uint8> Uncompressed_FromBinary;
	if( !URamaSaveUtility::DecompressFromFile(FileName,Uncompressed_FromBinary))
	{
		//File could not be loaded!
		VSCREENMSG("Rama Save System ~ File was found but could not be loaded! " + FileName );
		return 0;
	}
	
	FileIOSuccess = true;
	 
	//Reader
	FMemoryReader MemoryReader(Uncompressed_FromBinary, true);
	
	//! #1
	// Read version for this file format
	int32 SavegameFileVersion;
	MemoryReader << SavegameFileVersion;
    
	//! #2
	// Read engine and UE4 version information
	FPackageFileVersion SavedUE4Version;
	MemoryReader << SavedUE4Version;

	FEngineVersion SavedEngineVersion;
	MemoryReader << SavedEngineVersion;
	
	//!#3 Level Streaming, have to process
	StreamingLevelsStates.Empty();
	
	StreamingLevelsStates.Add("Old File Version, please resave file to save streaming level state as part of the file!" + FString("=true")); //So it will parse as expected
	if(SavegameFileVersion > 3)
	{
		StreamingLevelsStates.Empty();
		
		//Load Streaming Levels!
		MemoryReader << StreamingLevelsStates;
	}
	 
	return StreamingLevelsStates.Num();
}

void URamaSaveLibrary::GetAllRamaSaveComponents(UObject* WorldContextObject, TArray<URamaSaveComponent*>& RamaSaveComponents, FString GetOnlyStreamingLevelName, bool IgnoreStreamingActorsIfNoStreamingLevelSpecified)
{
	TArray<FString> SaveTags;
	GetAllRamaSaveComponentsWithTags(WorldContextObject, SaveTags,RamaSaveComponents,GetOnlyStreamingLevelName,IgnoreStreamingActorsIfNoStreamingLevelSpecified);
}
 
void URamaSaveLibrary::GetAllRamaSaveComponentsWithTags(
	UObject* WorldContextObject, 
	const TArray<FString>& SaveTags,  
	TArray<URamaSaveComponent*>& RamaSaveComponents, 
	FString GetOnlyStreamingLevelName,
	bool IgnoreStreamingActorsIfNoStreamingLevelSpecified)
{
	RamaSaveComponents.Empty();
	
	if(!WorldContextObject) return;
	 
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return;
	
	for(TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{
		if(!IsValid(*Itr)) continue;

		URamaSaveComponent* SaveComp = Itr->FindComponentByClass<URamaSaveComponent>();
		if(!SaveComp) continue;
		if(!SaveComp->IsValidLowLevel()) continue;
		if(!IsValid(SaveComp)) continue;
 
		if(IgnoreStreamingActorsIfNoStreamingLevelSpecified)
		{
			//If no streaming, SKIP all that are not persistent level
			if(GetOnlyStreamingLevelName == "")
			{
				AActor* Owner = SaveComp->GetOwner();
				if(!Owner) continue;
				 
				ULevel* Level = Owner->GetLevel();
				if(!Level) continue;
				if(!Level->IsPersistentLevel())
				{
					continue;
				}
			}
			//If streaming, make sure matches
			else
			{		  
				FString LevelPackageName = SaveComp->ActorStreamingLevel;
				
				if(GetOnlyStreamingLevelName != LevelPackageName)
				{
					continue;
					//~~~~~~~
				}
			}
		}
		
		//Save all streaming along with persistent
		//Legacy case
		else
		{
			//Streaming Level Filter
			if(GetOnlyStreamingLevelName != "")
			{
				FString LevelPackageName = SaveComp->ActorStreamingLevel;
				  
				if(GetOnlyStreamingLevelName != LevelPackageName)
				{
					continue;
					//~~~~~~~
				}
			}
		}
		
		//Tags Filter
		if(SaveTags.Num() > 0)
		{
			if(SaveComp->RamaSave_HasAnyOfSaveTags(SaveTags))
			{
				RamaSaveComponents.Add(SaveComp);
			}
		} 
		 
		//Add all if no tags were provided!
		else
		{
			RamaSaveComponents.Add(SaveComp);
		}
	}
	
	//Object Iterator even with World Compare finds twice as many placed Rama Save Components
	 
	/*
	for(TObjectIterator<URamaSaveComponent> Itr; Itr; ++Itr)
	{
		//Compare World to verify not a default object or viewport actor or other irrelevant actor
		if(Itr->GetWorld() != World) continue;
		if(!Itr->IsValidLowLevel()) continue;
		if(Itr->GetOwner()->IsPendingKill()) continue;
		
		RamaSaveComponents.Add(*Itr);
	}
	*/
}
void URamaSaveLibrary::GetAllRamaSaveActors(UObject* WorldContextObject, TArray<AActor*>& RamaSaveActors)
{
	TArray<FString> SaveTags;
	GetAllRamaSaveActorsWithTags(WorldContextObject,SaveTags,RamaSaveActors);
}

void URamaSaveLibrary::GetAllRamaSaveActorsWithTags(UObject* WorldContextObject, const TArray<FString>& SaveTags, TArray<AActor*>& RamaSaveActors)
{
	RamaSaveActors.Empty();
	
	if(!WorldContextObject) return;
	 
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return;
	//~~~~~~~~~~~
	
	for(TActorIterator<AActor> Itr(World); Itr; ++Itr)
	{ 
		if(!IsValid(*Itr)) continue;

		URamaSaveComponent* SaveComp = Itr->FindComponentByClass<URamaSaveComponent>();
		if(!SaveComp) continue;
		if(!SaveComp->IsValidLowLevel()) continue;
		if(!IsValid(SaveComp)) continue;
		 
		if(SaveTags.Num() > 0)
		{  
			if(SaveComp->RamaSave_HasAnyOfSaveTags(SaveTags))
			{
				RamaSaveActors.Add(*Itr);
			}
		}
		//Add all if no tags were provided!
		else
		{
			RamaSaveActors.Add(*Itr);
		}
	}
}
void URamaSaveLibrary::RamaSave_ClearLevel(UObject* WorldContextObject, bool DontDestroyPlayers, FString ClearOnlyStreamingLevel)
{
	if(!WorldContextObject) return;
	 
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if(!World) return;
	 
	TArray<AActor*> ToDestroy;
	for(TObjectIterator<URamaSaveComponent> Itr; Itr; ++Itr)
	{
		//Compare World to verify not a default object or viewport actor or other irrelevant actor
		if(Itr->GetWorld() != World) continue;
		if(!Itr->IsValidLowLevel()) continue;
		if(!IsValid(Itr->GetOwner())) continue;
		 
		//Streaming Levels Filter
		if(ClearOnlyStreamingLevel != "")
		{
			FString LevelPackageName = Itr->GetActorStreamingLevelPackageName();
			if(LevelPackageName != ClearOnlyStreamingLevel)
			{
				//Dont destroy actor!
				continue;
			}
		}
		  
		//! FGUID Special Case
		if(Itr->RamaSave_PersistentActorUniqueID.IsValid())
		{
			if(Itr->RamaSave_LogPersistentActorGUID) UE_LOG(RamaSave,Log,TEXT("RamaSave ~ Loading ~ Not destroying actor who has a valid RamaSave_PersistentActorUniqueID %s"), *Itr->GetOwner()->GetName());
			continue;
			//~~~~~~~
		}
		ToDestroy.Add(Itr->GetOwner());
	}
	
	for(AActor* Each : ToDestroy)
	{	 
		URamaSaveComponent* SaveComp = Each->FindComponentByClass<URamaSaveComponent>();
		if(!SaveComp) continue;
		if(!SaveComp->DestroyBeforeLoad) continue;
		
		APawn* Pawn = Cast<APawn>(Each);
		bool IsPlayer = Pawn && Pawn->GetPlayerState();
		
		if(IsPlayer && DontDestroyPlayers)
		{
			continue;
		}
		
		//Call PreDestroy
		if(SaveComp) 
		{
			if(SaveComp->RamaSaveEvent_PreLoadDestroy.IsBound())
			{ 
				SaveComp->RamaSaveEvent_PreLoadDestroy.Broadcast(SaveComp);
			}	
		}
		
		//Actor Destroy
		Each->Destroy();
		
		URamaSaveUtility::VDestroy(SaveComp);  	//Component hangs around otherwise
	}  
	 
	//!FF only, and GC full purge also doesnt help the demo net duplicate issue
	//Trigger GC purge, needs GC afterward to avoid net replays getting duplicates
	//World->ForceGarbageCollection(true); //full purge
	 
	//VSCREENMSGF("To Destroy Count is", ToDestroy.Num());
}
