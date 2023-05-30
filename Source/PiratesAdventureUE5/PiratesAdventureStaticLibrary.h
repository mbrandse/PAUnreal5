// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PiratesAdventureStaticLibrary.generated.h"

UENUM(BlueprintType)
enum class EDamageWeight : uint8
{
	DW_LIGHT UMETA(DisplayName = "Light"),
	DW_HEAVY UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EDamageCategory : uint8
{
	DC_GROUND UMETA(DisplayName = "Ground"),
	DC_AIR UMETA(DisplayName = "Air"),
	DC_VOLUME UMETA(DisplayName = "Volume"),
	DC_SPIKES UMETA(DisplayName = "Spikes")
};

UENUM(BlueprintType)
enum class EPlayerMovementDirection : uint8
{
	MD_NONE UMETA(DisplayName = "No Direction"), //Just in case
	MD_LEFT UMETA(DisplayName = "Left"),
	MD_RIGHT UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EPlayerToolType : uint8
{
	//This type should not be assigned to anything. It's so we can return NULLs.
	TOOL_NONE UMETA(DisplayName = "None"),
	TOOL_GUN UMETA(DisplayName = "FlintLock"),
	TOOL_LANTARN UMETA(DisplayName = "Lantarn"),
	TOOL_EYE UMETA(DisplayName = "Eye"),
	TOOL_GRENADE UMETA(DisplayName = "Grenade"),
	TOOL_WAND UMETA(DisplayName = "Wand")

	/**
	//Normal consumables you can stockpile are also considered to be tools. Element is always default.
	TOOL_ITEM UMETA(DisplayName = "Item"),
	//Necessary for making an HUD entry for the sword as well. The sword will actually never be called, so the template field can be left empty.
	TOOL_SWORD UMETA(DisplayName = "Sword")
	**/
};

UENUM(BlueprintType)
enum class EPlayerElementType : uint8
{
	//This type should not be assigned to anything. It's so we can return NULLs.
	ELEMENT_NONE UMETA(DisplayName = "None"),
	ELEMENT_FIRE UMETA(DisplayName = "FireSkill"),
	ELEMENT_ICE UMETA(DisplayName = "IceSkill"),
	ELEMENT_WIND UMETA(DisplayName = "WindSkill"),
	ELEMENT_WOOD UMETA(DisplayName = "WoodSkill"),
	ELEMENT_DEFAULT UMETA(DisplayName = "Default")
};

USTRUCT(Blueprintable)
struct FDamageAnimation
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		EDamageWeight DamageWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		EDamageCategory DamageCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		FVector AirImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		FVector GroundImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float RecoveryTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		int32 HitCount;
};

UENUM()
enum class EDamagePenetration : uint8
{
	/** The entire explosion will ignore any objects that can stop damage and just deal damage to anything within range */
	DP_PenetrateAll UMETA(DisplayName = "Penetrate All"),
	/** The core of the explosion will damage everything within its range, but everything else will be subject to the damage prevention channel */
	DP_PenetrateCoreOnly UMETA(DisplayName = "Penetrate Core Only"),
	/** The outer ring of the explosion will damage everything within its range, but the core will be subject to the damage prevention channel */
	DP_PenetrateOuterOnly UMETA(DisplayName = "Penetrate Outer Only"),
	/** The entire explosion will be subject to the damage prevention channel */
	DP_PenetrateNone UMETA(DisplayName = "Penetrate None")
};

//--------------------------------------------------------------------------------------------------------
// STRUCTS FOR SAVE DATA, SINCE WE DON'T WANT ITEM ASSETS FOR EVERYTHING
//--------------------------------------------------------------------------------------------------------

USTRUCT(Blueprintable)
struct FSaveDataPlayerElement
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Element")
		EPlayerElementType ElementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Element")
		bool bUnlockedElement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Element")
		bool bElementIsInactive;
};

USTRUCT(Blueprintable)
struct FSaveDataPlayerTool
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Tool")
		EPlayerToolType ToolType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Tool")
		bool bUnlockedTool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Tool")
		bool bUnlockedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Tool")
		bool bUnlockedDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Tool")
		bool bToolIsInactive;
};

UENUM(BlueprintType)
enum class ELeverState : uint8
{
	LEVER_NEUTRAL UMETA(DisplayName = "Neutral"),
	LEVER_LEFT UMETA(DisplayName = "Left"),
	LEVER_RIGHT UMETA(DisplayName = "Right")
};

USTRUCT(Blueprintable)
struct FSaveDataOverworld
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Overworld")
		FGuid GUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Switch")
		bool bHasBeenDiscovered;

	//Add variables here for unlockables (type, how many unlockables and how many unlocked. Needed for UI)
};

USTRUCT(Blueprintable)
struct FSaveDataSwitch
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Switch")
		FGuid GUID;
	
	//Switch (pressure, button)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Switch")
		bool bActivated;

	//Lever
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Switch")
		ELeverState LeverState;

	//Roll Lever
	//The roll lever is a lever where the player has to rotate the analog stick. The lock can be set sometimes to make the setting permanent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Switch")
		bool bLocked;
};

USTRUCT(Blueprintable)
struct FSaveDataDoor
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Door")
		FGuid GUID;

	//Unlocked simply means if it was associated with a key, it certainly isn't anymore.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Door")
		bool bUnlocked;

	//Open means opened. This could be through various means, but mostly due to autolocks that don't require keys. Like enemy arenas. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Door")
		bool bOpen;
};

USTRUCT(Blueprintable)
struct FSaveDataLadder
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Ladder")
		FGuid GUID;

	//Unlocked simply means that the ladder has been let down from the top floor, opening up access to the lower floor.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Ladder")
		bool bUnlocked;
};

USTRUCT(Blueprintable)
struct FSaveDataElevator
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Elevator")
		FGuid GUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data || Elevator")
		int32 Floor;
};

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPiratesAdventureStaticLibrary : public UObject
{
	GENERATED_BODY()
	
};
