// Copyright 2018 by Nathan "Rama" Iyer. All Rights Reserved.
#pragma once

#include "RamaSaveEngine.h"

#include "Engine/DeveloperSettings.h"
#include "RamaSaveSystemSettings.generated.h"

/** Used with the Rama Save System to remap old variable and class names to your new names to ensure old save files work with updated game versions <3 Rama */
USTRUCT(BlueprintType)
struct FRamaSaveSystemAssetPathRemap
{
	GENERATED_USTRUCT_BODY()
 
	/*
		Classes: Visit Project Settings -> Rama Save System and turn on LogLoadedActorClassPath to see what the old class path might be, and check log after loading any save file 
		Vars: Just the variable name itself, the old one, whose name changed.
		
		Check the log to see where I've indicated an old property / variable could not be loaded from an old save file. 
		
		-Rama
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Joy Color")
	FString OldPath = "";
	
	/*
		Classes: Right-click any blueprint / class and choose Copy Asset Reference to see what new path might be.
		Vars: Just the variable name itself, the new one, whose name changed.
		
		Check the log to see where I've indicated an old property / variable could not be loaded from an old save file. 
		
		-Rama
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Joy Color")
	FString NewPath = "";
};

/**
 *  <3 Rama
 */
UCLASS(config=Game, defaultconfig)
class URamaSaveSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URamaSaveSystemSettings()
	{ 
		SectionName = "~ Rama Save System ~";
	} 
	
	/** 
		This is a float to support version numbers like "2.18"; if you want to verify a save file against certain key builds I recommend your version values always only increase as you make more of them.
		
		-Rama
	*/
	UPROPERTY(config, Category = "Rama Save System", EditAnywhere, BlueprintReadWrite)
	float GameVersion = 1.0;
	
	UPROPERTY(config, Category = "Core", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ARamaSaveEngine> RamaSaveEngineBP = ARamaSaveEngine::StaticClass();
	
	UPROPERTY(config, Category = "Core", EditAnywhere, BlueprintReadWrite, meta=(TitleProperty="NewPath"))
	TArray<FRamaSaveSystemAssetPathRemap> RemappedVariablesAndClasses;
	
	void GetRemappedPath(FString& Path) const
	{
		for(const FRamaSaveSystemAssetPathRemap& Each : RemappedVariablesAndClasses)
		{
			if(Each.OldPath == Path)
			{
				Path = Each.NewPath;
				return;
			}
		}
	}
	
	/** 
		+ Adds Support For UE4 "SaveGame" Property
		
		+ Adds Support For Actor Subcomponents With Same Property Name
		
		~~~ Important Note ~~~
		Save files that have this value true/false are incompatible, so only set this to true if you don't need to be able to load old save files / have your users load old save file data.
		
		Setting this value to true adds the new features listed above!
		
		~~~ Overview ~~~
		In my original implementation any properties of the owning Actor or other components HAD to be put in a string variable list.
		
		Now with the new setup you have the option to simply mark any properties you want to save with UE4's SaveGame meta data in C++ or the property tag in Blueprints.
		
		If your project was using both my save system and UE4's save system, you will probably want to revert to the old behavior by setting this to false.
		
		~~~ Saving Several Subcomponents of Actor that share a property with the same Name ~~~
		
		If you have several subcomponents of an actor that had the same exact property name like "Location" or "Health", you can now save these same-name vars per-component with this new setup!
		
		~~~ How to Use ~~~
		You can easily save subcomponent properties of an actor where several components have a property with the same name by setting the SaveGame flag on the relevant properties, or by adding Pre-existing properties to the RamaSave_ComponentVarsToSave list :)
		
		<3 Rama
	*/
	UPROPERTY(config, Category = "Core", EditAnywhere, BlueprintReadWrite)
	bool SaveAllPropertiesMarkedAsSaveGame = true;
	
	/**
		Should save files be compressed? 
		
		Turn this off if experiencing file-read errors with compressed data files.
		
		Please note, I do check if a file is compressed on load, even when this feature is turned on, 
		and so I will still attempt to decompress old save files, but new files will be saved in uncompressed fashion.
		
		IF a user is experiencing an issue with a compressed save file:
		
		They will have to save a fresh, uncompressed save file after this feature is turned on in their build, 
		in order to experience a different result.
		
		♥
		
		Rama
	*/
	UPROPERTY(config, Category = "Core", EditAnywhere, BlueprintReadWrite)
	bool UseUncompressedSaveFiles = false;
	
	/** 
		If true, you can have many components of an actor with same variable name and if you incude that variable name in the RamaSave_ComponentVarsToSave list then it will be saved for every component!
		
		This is a recommended feature if you are still in development and don't need to keep old save files :)
	*/
	//! In Progress
	//UPROPERTY(config, Category = "Game", EditAnywhere, BlueprintReadWrite)
	//bool RamaSave_SupportSavingSubcomponentPropertiesWithSameName = true;
	
	/** If false, entire saving of world is done in one loop and may cause a hitch for large worlds */
	UPROPERTY(config, Category = "Async Save", EditAnywhere, BlueprintReadWrite)
	bool AsyncSave = false;
	 
	/** You can customize the interval of time between processing of chunks <3 Rama */
	UPROPERTY(config, Category = "Async Save", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "AsyncSave"))
	float AsyncSaveTickInterval = 0.01;
	 
	/** How many actors should be processed per tick? */
	UPROPERTY(config, Category = "Async Save", EditAnywhere, BlueprintReadWrite, meta = (editcondition = "AsyncSave"))
	float AsyncSaveActorChunkSize = 1;
	
	/**
		Unchecking this can increase the speed of saving in cases where you have actors with tons of variables that you've added yourself.
		
		It is important if you are new to my save system to leave this checked until you understand the type of objects that can and can't be saved. 
		
		Any asset object like a material asset reference or mesh asset ref can be saved. 
		
		But any objects created during runtime cannot be saved in their entirety, such as a material instance dynamic.
		
		To save/load something like a runtime mesh or material instance dynamic, save the basic parameters in a global save actor and then during load, reconstruct the dynamically created object from the simple data values.
	*/	
	UPROPERTY(config, Category = "Performance", EditAnywhere, BlueprintReadWrite)
	bool Saving_PerformObjectValidityChecks = true;
	
	/** 
		If you want to use Level Streaming make sure this checked / on / true / gooo!
		
		Unchecking this has a chance to improve performance
	*/
	//UPROPERTY(config, Category = "Game", EditAnywhere, BlueprintReadWrite)
	//bool Saving_LevelStreaming_PerformActorPackageLookup = true;
	/*
		//RamaSaveComponent.cpp
		
		//! 4.7333 Actor Tags
		Ar << RamaSave_SaveTags;
		
		//Set the Level Package Name, so that can filter when loading if that is desired
		LevelPackageName = GetActorStreamingLevelPackageName(); //<~~~~
	*/
	
	
	/** If you need extensive debug information printed to UE4 Log during the loading process of a file, use this! If this is true, log info will be printed regardless of per-Rama-Save-Component log verbosity settings. <3 Rama */
	UPROPERTY(config, Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool Loading_GlobalVerboseLogging = false;
	
	/** If this is true, all loading of physics data from file will be disabled, primarly for debug purposes so you don't have to turn physics serialization off in each individual actor blueprint while testing */
	UPROPERTY(config, Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool Loading_GlobalDisablePhysicsLoad = false;
	
	/** If this is true and you save a RamaSaveObject as StaticData along with your save file, then each property being saved/loaded will print to log! */
	UPROPERTY(config, Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool LogSavingAndLoadingOfEachStaticDataProperty = false;
	
	/** Log the names of actor class paths as stored in the file to make it easier to figure out how you would need to remap the path of a class whose name changed after the file was saved. <3 Rama */
	UPROPERTY(config, Category = "Debug", EditAnywhere, BlueprintReadWrite)
	bool LogLoadedActorClassPath = false;
	
public:
	static URamaSaveSystemSettings* Get()
	{  
		return GetMutableDefault<URamaSaveSystemSettings>();
	}
};
