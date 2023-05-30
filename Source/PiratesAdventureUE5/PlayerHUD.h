// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

UENUM(BlueprintType) //RingType
enum class ERingMenuType : uint8
{
	RMT_None UMETA(DisplayName = "NULL"),
	RMT_Element UMETA(DisplayName = "Element Ring"),
	RMT_Tool UMETA(DisplayName = "Tool Ring"),
	RMT_Misc UMETA(DisplayName = "Misc Ring") //The misc ring will be used for any ring that is not tool or element.
};

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:

	//We need this variable due to the ability to switch to an AI controller (at which point we lose the normal way of referencing the character).
	UPROPERTY()
		class APlayerCharacter* HUDOwner;

	virtual void BeginPlay();

	//virtual void DrawHUD() override;

	//--------------------------------------------------------------------------------------------------------
	// HUD ELEMENTS
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Hud Elements")
		TSubclassOf<class UPlayerHUDRingMenu> ElementRingTemplate;

	UPROPERTY(BlueprintReadOnly)
		class UPlayerHUDRingMenu* ElementRingInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Hud Elements")
		TSubclassOf<class UPlayerHUDRingMenu> ToolRingTemplate;

	UPROPERTY(BlueprintReadOnly)
		class UPlayerHUDRingMenu* ToolRingInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Hud Elements")
		TSubclassOf<class UPlayerHUDHealth> HealthTemplate;

	UPROPERTY(BlueprintReadOnly)
		class UPlayerHUDHealth* HealthInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Hud Elements")
		TSubclassOf<class UPlayerHUDCharge> ChargeTemplate;

	UPROPERTY(BlueprintReadOnly)
		class UPlayerHUDCharge* ChargeInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true), Category = "Hud Elements")
		TSubclassOf<class UPlayerHUDContextPopup> ContextPopupTemplate;

	UPROPERTY()
		class UPlayerHUDContextPopup* ContextPopupInstance;


	//--------------------------------------------------------------------------------------------------------
	// CONTEXT FUNCTIONS
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Action")
		void SetContextText(const FText& ContextText, FLinearColor ContextColor = FColor::White);

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Action")
		void ClearContextText();

	//--------------------------------------------------------------------------------------------------------
	// HUD CONTROL
	//--------------------------------------------------------------------------------------------------------
	
	//so we can wrestle control away from the pawn, without having to do a state change. 
	//Since UI can be called anywhere, it's risky to make a state out of it.
	UFUNCTION(BlueprintCallable, Category = "Input")
		void EnableHUDInput(class APlayerController* PlayerController, bool bResetBindings = false);

	//Hud related input functions
	virtual void InterfaceAcceptPress();
	virtual void InterfaceAcceptRelease();
	virtual void InterfaceCancelPress();
	virtual void InterfaceCancelRelease();
	virtual void InterfaceUpPress();
	virtual void InterfaceUpRelease();
	virtual void InterfaceLeftPress();
	virtual void InterfaceLeftRelease();
	virtual void InterfaceDownPress();
	virtual void InterfaceDownRelease();
	virtual void InterfaceRightPress();
	virtual void InterfaceRightRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Accept Press"))
		void ReceiveInterfaceAcceptPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Accept Release"))
		void ReceiveInterfaceAcceptRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Cancel Press"))
		void ReceiveInterfaceCancelPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Cancel Release"))
		void ReceiveInterfaceCancelRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Up Press"))
		void ReceiveInterfaceUpPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Up Release"))
		void ReceiveInterfaceUpRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Left Press"))
		void ReceiveInterfaceLeftPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Left Release"))
		void ReceiveInterfaceLeftRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Down Press"))
		void ReceiveInterfaceDownPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Down Release"))
		void ReceiveInterfaceDownRelease();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Right Press"))
		void ReceiveInterfaceRightPress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "Right Release"))
		void ReceiveInterfaceRightRelease();


	//--------------------------------------------------------------------------------------------------------
	// RING CONTROL
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintImplementableEvent, Category = "Ring Menu")
		void SwitchEquipVisualization(ERingMenuType RingMenu);

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Action")
		void ResetEquipVisualization();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ring Menu")
		void ProcessKeyBinding(FName KeyBinding);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ring Menu")
		void SetRingUnlocked(class UDataAssetItem* Item);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ring Menu")
		void SetRingInactive(class UDataAssetItem* Item);

	////DEBUG DELETE LATER
	//TSharedPtr<class SLoadingScreenStart> MenuWidget;
	
};
