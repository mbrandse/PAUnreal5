// Copyright 2018 by Nathan "Rama" Iyer. All Rights Reserved.
#pragma once

//Serializaiton
#include "Runtime/Core/Public/Serialization/Archive.h"
#include "Runtime/Core/Public/Serialization/MemoryWriter.h"
#include "Runtime/Core/Public/Serialization/NameAsStringProxyArchive.h"
#include "Runtime/CoreUObject/Public/Serialization/ObjectAndNameAsStringProxyArchive.h"

//Level Streaming
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"

//This Plugin
#include "RamaSaveObject.h"
#include "RamaSaveComponent.h"
#include "RamaSaveEngine.generated.h"
 
//Version
#define JOY_SAVE_VERSION 7

#define JOY_SAVE_VERSION_STREAMINGLEVELS 			4
#define JOY_SAVE_VERSION_MULTISUBCOMPONENT_SAMENAME 5
#define JOY_SAVE_VERSION_SAVEOBJECT 				6
#define JOY_SAVE_VERSION_CUSTOM_USER_VERSION 		JOY_SAVE_VERSION

DECLARE_STATS_GROUP(TEXT("RamaSave"), STATGROUP_RamaSave, STATCAT_Advanced);

//Pirates Adventure addition!
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreLoadFinishedSignature, FString, FileName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFullLoadFinishedSignature, FString, FileName);

USTRUCT()
struct FRamaSaveEngineParams
{
	GENERATED_USTRUCT_BODY()
 
	UPROPERTY()
	TArray<FString> LoadOnlyActorsWithSaveTags;
	
	UPROPERTY()
	FString FileName = "";
	
	UPROPERTY()
	bool DestroyActorsBeforeLoad = true;
	
	UPROPERTY()
	bool DontLoadPlayerPawns = false;
	
	UPROPERTY()
	FString LoadOnlyStreamingLevel = "";
	
};

USTRUCT()
struct FRamaAsyncSaveUnit
{
	GENERATED_USTRUCT_BODY()
 
	UPROPERTY()
	bool UpdateUI = true;
	
	UPROPERTY()
	int32 Index = 0;
	
	UPROPERTY()
	int32 TotalComponents = 0;
	
	UPROPERTY()
	TArray<URamaSaveComponent*> RamaSaveComponents;
	
	UPROPERTY()
	FString FileName = "";
	
	UPROPERTY()
	TArray<uint8> ToBinary;
	
	int64 PrevPos = 0;
	
	UPROPERTY()
	bool Finished = false;
	
	UPROPERTY()
	bool CompressionStarted = false;
	
	void ClearAsyncArchive(){}
};

UCLASS()
class ARamaSaveEngine	: public AActor
{
	GENERATED_BODY()
public:
	ARamaSaveEngine(const FObjectInitializer& ObjectInitializer);
	
	/** Save process will be cancelled for a streaming level if it is pending a state change like unloading or hiding */
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void SaveCancelledForStreamingLevel(const FString& FileName);
	
	/** When loading a level that includes streaming levels, this event will be called only after all streaming levels are loaded. If no streaming levels are involved it is instantaneous. 
		This event runs __before__ all Rama Save Components have called ActorFullyLoaded. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void LoadProcessFinished_PreActorFullyLoaded(const FString& FileName);
	
	/** When loading a level that includes streaming levels, this event will be called only after all streaming levels are loaded. If no streaming levels are involved it is instantaneous. 
		This event runs __after__ all Rama Save Components have called ActorFullyLoaded. */
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void LoadProcessFinished(const FString& FileName);

	//PIRATES ADVENTURE ADDITION
	FOnPreLoadFinishedSignature OnPreLoadFinished;
	FOnFullLoadFinishedSignature OnFullLoadFinished;
	
//Delegates / Events
public:
	/** Value proceeds from 0 to 1 during async save <3 Rama */
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void Async_ProgressUpdate(const FString& FileName, float Progress);
	 
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void Async_SaveStarted(const FString& FileName);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void Async_SaveFinished(const FString& FileName);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Rama Save System")
	void Async_SaveCancelled(const FString& FileName);
	
	/** C++ hooks run _before_ the BP assignable events */
	virtual void Async_ProgressUpdate_CPP(const FString& FileName, float Progress) {}
	virtual void Async_SaveStarted_CPP(const FString& FileName) {}
	virtual void Async_SaveFinished_CPP(const FString& FileName) {}
	virtual void Async_SaveCancelled_CPP(const FString& FileName) {}
	
//Saving
public:
	
	static void RamaSave_SaveStaticData(FString FileName, bool& FileIOSuccess, URamaSaveObject* StaticSaveData);
	
	//SYNC
	void RamaSave_SaveToFile(FString FileName, bool& FileIOSuccess, bool& AllComponentsSaved, TArray<FString> InSaveOnlyActorsWithTags, FString SaveOnlyStreamingLevel="", bool IgnoreStreamingActorsIfNoLevelSpecified=false,URamaSaveObject* StaticSaveData = nullptr);
	
	UPROPERTY()
	TArray<URamaSaveComponent*> RamaSaveComponents;
	
	UPROPERTY()
	TArray<FString> SaveOnlyActorsWithTags;
	
	//~~~
	//ASYNC
	
	UPROPERTY()
	TArray< FRamaAsyncSaveUnit > AsyncUnits;
	void RamaSaveAsync(FRamaAsyncSaveUnit& AsyncUnit);
	
	void RamaSave_SaveToFile_ASYNC(FString FileName, bool& FileIOSuccess, bool& AllComponentsSaved, FString SaveOnlyStreamingLevel="",bool IgnoreStreamingActorsIfNoLevelSpecified=false, URamaSaveObject* StaticSaveData = nullptr);
	
	bool RamaSaveAsync_SaveChecks = false;
	int32 RamaSaveAsync_ChunkGoal = 1;
	
	FTimerHandle TH_RamaSaveAsync;
	void RamaSaveAsync();
	
	//Returns true if a save was in progress and was cancelled
	bool RamaSaveAsync_Cancel();
	
//Loading
public:
	UPROPERTY()
	FRamaSaveEngineParams LoadParams;
	
	//Unload/Load appropriate Levels
	void Phase1(const FRamaSaveEngineParams& Params, bool HandleStreamingLevelsLoadingAndUnloading);
	
	float AsyncStartTime = 0;
	
	UPROPERTY()
	TArray<ULevelStreaming*> Load_StreamingLevels;
	
	FTimerHandle TH_AsyncStreamingLoad;
	void AsyncStreamingLoad();
	
	//Loooooooaaaaaaadddddd!!!
	void Phase2();
	
	//What file version is being loaded?
	static int32 LoadedSaveVersion;
	
	
	static void SaveStaticData(FArchive& Ar, URamaSaveObject* StaticData);
	static void SkipStaticData(FArchive& Ar);
	
	static URamaSaveObject* LoadStaticData(bool& FileIOSuccess,  FString FileName);
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

