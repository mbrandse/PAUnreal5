// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "PiratesAdventureStaticLibrary.h"
#include "PlayerToolBase.generated.h"

UENUM(BlueprintType)
enum class EChargeState : uint8
{
	CS_NONE UMETA(DisplayName = "None"),
	CS_GODMODE UMETA(DisplayName = "God Mode"),
	CS_DELAY UMETA(DisplayName = "Delay Charge"),
	CS_DEPLETECHARGE UMETA(DisplayName = "Depleting Charge"),
	CS_RECOVERCHARGE UMETA(DisplayName = "Recovering Charge"),
	CS_PENALTYCHARGE UMETA(DisplayName = "Penalty Charge") //penalty charge means that if the tool hit 0 or lower, we have to wait until its recharged entirely before we can use it again
};

UENUM(BlueprintType)
enum class EToolUsageType : uint8
{
	TUT_INSTANT UMETA(DisplayName = "Instant Type Tool"),
	TUT_CHARGE UMETA(DisplayName = "Charge Type Tool")
};

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPlayerToolBase : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:

	UPlayerToolBase(const FObjectInitializer& ObjectInitializer);

	//--------------------------------------------------------------------------------------------------------
	// TOOL PROPERTY VALUES
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "Tool Properties")
		EPlayerToolType ToolType;

	UFUNCTION(BlueprintPure, Category = "Player Tool")
		EPlayerToolType GetToolType() { return ToolType; };

	UPROPERTY()
		class APlayerCharacter* ToolOwner;

	UFUNCTION(BlueprintPure, Category = "Player Tool")
		class APlayerCharacter* GetToolOwner() { return ToolOwner; };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
		class UDataAssetItem* ItemData;

	//--------------------------------------------------------------------------------------------------------
	// TOOL REGISTRY AND ACTIVATION
	//--------------------------------------------------------------------------------------------------------

	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Tool Register"))
		void ReceiveToolRegister(APlayerCharacter* Owner);

	virtual void InitializeTool();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Tool Equip"))
		void ReceiveInitializeTool();

	virtual void DeinitializeTool();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Tool Unequip"))
		void ReceiveDeinitializeTool();

	//--------------------------------------------------------------------------------------------------------
	// SETTING ELEMENT
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Player Tool")
		void SetToolElement(EPlayerElementType NewElement);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "Set Elemental Properties"))
		void OnReceiveSetToolElement(EPlayerElementType NewElement);

	UFUNCTION(BlueprintPure, Category = "Player Tool")
		EPlayerElementType GetToolElement();

	//--------------------------------------------------------------------------------------------------------
	// TOOL USAGE
	//--------------------------------------------------------------------------------------------------------
	
	//Used for aiming tools; when set to true, the player can start aiming
	UPROPERTY(BlueprintReadOnly, Category = "Tool Properties")
		bool bToolIsActive;

	UPROPERTY()
		bool bToolHasAiming;

	bool bIsCoolingDown; //Used for tools with ammunition, to make sure we don't reuse it while animations are still playing. 
	bool bLockToolAiming; //Stopgap measure for making sure the tool cannot aim while the player is turning during movement.

	//Activate and deactivate tool effect
	virtual void StartTool();
	virtual void StartToolNoAmmo();
	virtual void StopTool();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Start Tool"))
		void ReceiveStartTool();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On No Ammo"))
		void ReceiveStartToolNoAmmo();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Stop Tool"))
		void ReceiveStopTool();

	UFUNCTION(BlueprintPure, Category = "Player Tool")
		bool CanStartTool();

	UFUNCTION(BlueprintCallable, Category = "Player Tool", meta = (DisplayName = "Toggle Tool"))
		virtual void ToggleTool() { bToolIsActive = !bToolIsActive; };

	//--------------------------------------------------------------------------------------------------------
	// TOOL AIMING
	//--------------------------------------------------------------------------------------------------------

	float AimToolRight;
	float AimToolUp;
	bool bToolIsAiming;

	//starts or stops aiming tool. Doesn't activate it.
	virtual void StartAimTool(float X, float Y) {};
	virtual void StopAimTool() {};

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Start Aim Tool"))
		void ReceiveStartAimTool(float X, float Y);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Tool", meta = (DisplayName = "On Stop Aim Tool"))
		void ReceiveStopAimTool();

	UFUNCTION(BlueprintPure, Category = "Player Tool")
		void GetAim(float& X, float& Y) { X = AimToolRight; Y = AimToolUp; };

	UFUNCTION(BlueprintPure, Category = "Tool")
		bool ToolIsAiming() { return ToolUsageType == EToolUsageType::TUT_CHARGE && bToolIsAiming; };

	//For instantaneous effects for certain tools
	bool bToolIsBeingUsed;
	FTimerHandle ToolUsedHandle;

	//For instantaneous effects, for instance when we fire the gun "from the hip"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
		float ToolBeingUsedTimer = 0.1f;

	void TurnOffToolBeingUsed();

	//--------------------------------------------------------------------------------------------------------
	// TOOL CHARGING
	//--------------------------------------------------------------------------------------------------------

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//A failsafe for tools that need continuous ticking for their animation. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Properties")
		bool bShouldAlwaysTick;

	EChargeState ChargeState; //Now handles everything from delay and whatnot.
	EToolUsageType ToolUsageType;
	float AmmoCurrent;
	float ChargeDelayTimeElapsed; //required for at least the delayed state

	FTimerHandle StartRechargeHandle;
	void StartRecharge();

	//Do we need this? Especially now that we are using enums?
	//UFUNCTION(BlueprintCallable, Category = "Tool")
	//	void ToggleChargeDecrease(bool bDecreaseCharge);

	//TODO: add a single upgrade system for the ammocount and recharge time. All tools are now equal and can get a max of two upgrades
	// 1. Ammocount upgrade (+50%)
	// 2. Recharge time upgrade (-30%)
	
	//For charge tools, the ammo count is equal to duration. For instant tools, ammo count is max number of bullets.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tool Properties")
		float AmmoCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tool Properties")
		bool bAmmoCountUpgraded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tool Properties")
		float AmmoRechargeTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tool Properties")
		bool bAmmoRechargeTimeUpgraded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tool Properties")
		float AmmoRechargeDelay;

	//TODO: add a godmode that only affects certain elements. This likely needs to be done in the player
	UFUNCTION(BlueprintPure, Category = "Tool")
		bool GetAmmoIsInfinite();

	UFUNCTION(BlueprintPure, Category = "Tool")
		float GetCurrentAmmo() { return AmmoCurrent; };

	UFUNCTION(BlueprintCallable, Category = "Tool")
		void UpgradeAmmoCount() { bAmmoCountUpgraded = true; };

	UFUNCTION(BlueprintCallable, Category = "Tool")
		void UpgradeRechargeTime() { bAmmoRechargeTimeUpgraded = true; };

	UFUNCTION(BlueprintPure, Category = "Tool")
		bool ChargeIsDecreasing() { return ChargeState == EChargeState::CS_DEPLETECHARGE; };

	//--------------------------------------------------------------------------------------------------------
	// TOOL DEBUG
	//--------------------------------------------------------------------------------------------------------

	//Visualizes feedback for the sake of debugging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool Debug")
		bool bDebug = false;
};
