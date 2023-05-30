// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PiratesAdventureStaticLibrary.h"
#include "DataAssetItem.generated.h"

UENUM(Blueprintable)
enum class EItemCategory : uint8
{
	IC_Consumable UMETA(DisplayName = "Consumable"),
	IC_Key UMETA(DisplayName = "Key"),
	IC_Collectible UMETA(DisplayName = "Collectible"),
	IC_Equipment UMETA(DisplayName = "Equipment"),
	IC_Weapon UMETA(DisplayName = "Weapon"),
	IC_Tool UMETA(DisplayName = "Tool"),
	IC_Item UMETA(DisplayName = "Item")
};

USTRUCT(Blueprintable)
struct FInventoryItemData
{
	GENERATED_USTRUCT_BODY()

	//An unique name for the item, to make sure we can distinguish between all items. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FName ID;

	//The category for sorting purposes. We will likely add subcategories here, but we need to make a custom asset viewer for that probably.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		EItemCategory Category;

	//The name of the item, as it will be used in game. Can be identical to the names of other items, unlike ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FText NameText;

	//The help text that appears when this item is received. Space is less, so we need a separate version that's shorter.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FText SupportTextSmall;

	//The help text that is shown when the player asks for more information in the menu.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FText SupportText;

	//The material used for the icon of the item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		UMaterialInterface* Icon;

	//Physical item data: DO WE STILL NEED THIS? Especially considering the fact we are going to remove any meshes associated with items.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TSubclassOf<class AActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool bIsEmpty;

	//WEAPON DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		EPlayerElementType ElementType = EPlayerElementType::ELEMENT_NONE;

	//TOOL DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		EPlayerToolType ToolType = EPlayerToolType::TOOL_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TSubclassOf<class UPlayerToolBase> ToolTemplate;

	UPROPERTY()
		class UPlayerToolBase* Tool;

	//EQUIPMENT DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		class USkeletalMesh* MeshTemplate;

	//ALL ITEMS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FName RingMenuButtonAssociation;
};


/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UDataAssetItem : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
		FInventoryItemData ItemData;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
		int32 AmmoCount = 0;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	//	bool bUnlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool bRollover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool bActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool bInactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		class UMaterialInstanceDynamic* DynamicMaterial;
	
};
