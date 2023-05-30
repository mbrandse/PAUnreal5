// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerHUDElementBase.h"
#include "PlayerHUDRingMenu.generated.h"

UENUM()
enum class EButtonAssociation : uint8
{
	BUTTON_DPADUP UMETA(DisplayName = "D-pad Up"),
	BUTTON_DPADLEFT UMETA(DisplayName = "D-pad Left"),
	BUTTON_DPADDOWN UMETA(DisplayName = "D-pad Down"),
	BUTTON_DPADRIGHT UMETA(DisplayName = "D-pad Right"),
	BUTTON_SELECT UMETA(DisplayName = "Select")
};

USTRUCT(Blueprintable)
struct FRingMenuIcon
{
	GENERATED_USTRUCT_BODY()

public:

	class UImage* Image;

	class UCanvasPanelSlot* Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Angle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialInterface* MaterialTemplate;

	class UMaterialInstanceDynamic* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EButtonAssociation ButtonAssociation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bDefault = false;
};

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPlayerHUDRingMenu : public UPlayerHUDElementBase
{
	GENERATED_BODY()

public:

	
};
