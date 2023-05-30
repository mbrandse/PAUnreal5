// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PiratesAdventureStaticLibrary.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerStateType : uint8
{
	//GPS = Game Player State 
	//UPS = UI Player State

	GPS_None UMETA(DisplayName = "Game | None"), //default value, not used in classes. Doesn't have any functionality.
	GPS_GameNormal UMETA(DisplayName = "Game | Normal"),
	GPS_GameClimb UMETA(DisplayName = "Game | Climb"),
	GPS_GameShimmy UMETA(DisplayName = "Game | Shimmy"),
	GPS_GameLedgegrab UMETA(DisplayName = "Game | Ledge Grab"),
	GPS_GameWallJump UMETA(DisplayName = "Game | Wall Jump"),
	GPS_GameBoat UMETA(DisplayName = "Game | Boat"),
	GPS_GamePush UMETA(DisplayName = "Game | Push"),
	GPS_GameCarryThrow UMETA(DisplayName = "Game | Carry and Throw"),
	GPS_GameDamage UMETA(DisplayName = "Game | Damage"),
};

USTRUCT(BlueprintType)
struct FPlayerStateController
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EPlayerStateType StateID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UPlayerInteractionStateBase* StateClass;
};

USTRUCT(Blueprintable)
struct FContextActionData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
		class AActor* ContextActor;

	UPROPERTY()
		FText ContextText;

	UPROPERTY()
		FLinearColor ContextColor;

	UPROPERTY()
		bool bShowButton;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangeDirectionSignature, EPlayerMovementDirection, NewDirection);

UCLASS()
class PIRATESADVENTUREUE5_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//--------------------------------------------------------------------------------------------------------
	// COMPONENTS
	//--------------------------------------------------------------------------------------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class URamaSaveComponent* SaveSystemComponent;

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCineCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UPlayerSpringArmComponent* CameraBoom;

public:

	//--------------------------------------------------------------------------------------------------------
	// COMPONENT GETTERS
	//--------------------------------------------------------------------------------------------------------

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCineCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class UPlayerSpringArmComponent* GetCameraBoom() const { return CameraBoom; }

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//--------------------------------------------------------------------------------------------------------
	// SAVE FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION()
		void PlayerFullyLoaded(class URamaSaveComponent* RamaSaveComponent, FString LevelPackageName);

	//TODO: MOVE TO RAMA SAVE STATIC DATA
	FVector SavedLocationData;
	EPlayerMovementDirection SavedDirectionData;

	//--------------------------------------------------------------------------------------------------------
	// INPUT FUNCTIONS 
	//--------------------------------------------------------------------------------------------------------

	void PerformJump();

	void MoveRightLeft(float Value);
	void MoveUpDown(float Value);
	void LookRightLeft(float Value);
	void LookUpDown(float Value);

	void StartAttack();
	void StopAttack();
	void StartTool();
	void StartDefend();
	void StopDefend();
	//dodge will now be folded under start/stop defend

	void CrossUp();
	void CrossLeft();
	void CrossRight();
	void CrossDown();

	void StartContext();
	void StopContext();

	void ToggleStartMenu();
	void StartToolMenu(); 
	void StopToolMenu(); 
	void StartElementMenu();
	void StopElementMenu();
	void UIUp();
	void UILeft();
	void UIRight();
	void UIDown();

	//--------------------------------------------------------------------------------------------------------
	// PARENT CLASS IMPLEMENTATIONS
	//--------------------------------------------------------------------------------------------------------

	virtual void Landed(const FHitResult& Hit);

	UPROPERTY(BlueprintAssignable, Category = "Player Functions || Movement")
		FLandedSignature OnPlayerLand;

	//--------------------------------------------------------------------------------------------------------
	// STATE FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	TArray<FPlayerStateController> RegisteredStates;
	class UPlayerInteractionStateBase* CurrentState;
	EPlayerStateType CurrentStateType;

	void RegisterStateComponent(class UPlayerInteractionStateBase* NewState);
	void SwitchPlayerState(EPlayerStateType NewState);

	//Calls the End State on the current Player State and then switches to new state (or default if nothing can be found). 
	//If bForceStateReset is set, it will call Beginstate even if the state is currently the same.
	UFUNCTION(BlueprintCallable, Category = "Player Functions || Player State")
		void SetNewPlayerState(EPlayerStateType NewState, bool bForceStateReset = false);

	//Returns the state if it can be found in the registered states. Otherwise returns NULL
	UFUNCTION(BlueprintPure, Category = "Player Functions || Player State")
		class UPlayerInteractionStateBase* GetRegisteredState(EPlayerStateType State);

	//Returns the current statetype. May be useful for things like animation blueprints
	UFUNCTION(BlueprintPure, Category = "Player Functions || Player State")
		EPlayerStateType GetCurrentPlayerState() { return CurrentStateType; };

	//--------------------------------------------------------------------------------------------------------
	// GENERAL MOVEMENT - DUCK AND CROUCH
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY()
		bool bIsDucked;

	//Due to the StartCrouch and EndCrouch functions also modifying the collision capsule offset, we don't want to call those in BP anymore. 
	//Instead, we will use the 4 functions below to call in addition to start/endcrouch and use those 4 functions to set things like camera effects.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Functions || Movement")
		void OnPlayerDuck();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Functions || Movement")
		void OnPlayerUnduck();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Functions || Movement")
		void OnPlayerCrouch();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Functions || Movement")
		void OnPlayerUncrouch();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void ForceUnCrouch();

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		bool IsDucking() { return bIsDucked; };

	//--------------------------------------------------------------------------------------------------------
	// GENERAL MOVEMENT - JUMP
	//--------------------------------------------------------------------------------------------------------

	virtual void Jump();
	void DoubleJump();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		bool bCanDoubleJump;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		bool bIsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		bool bIsDoubleJumping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float JumpSingleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float JumpDoubleHeight;

	//--------------------------------------------------------------------------------------------------------
	// GENERAL MOVEMENT - MOVEMENT
	//--------------------------------------------------------------------------------------------------------

private:
	bool bDisableMovement = false;
	bool bDisableAutoRotation = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void DisableMovement();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void EnableMovement();

	UFUNCTION(BlueprintCallable, Category = "Morgan Functions || Movement")
		bool GetMovementDisabled() { return bDisableMovement; };

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void EnableAutoRotation() { bDisableAutoRotation = false; };

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void DisableAutoRotation() { bDisableAutoRotation = true; };

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void EnableAxisLock();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Movement")
		void DisableAxisLock();

	float VerticalMovementSpeedRaw;
	float VerticalMovementSpeed;
	float HorizontalMovementSpeedRaw;
	float HorizontalMovementSpeed;
	float VerticalAimSpeedRaw;
	float VerticalAimSpeed;
	float HorizontalAimSpeedRaw;
	float HorizontalAimSpeed;
	EPlayerMovementDirection LastKnownDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float DeadZoneMovementLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float DeadZoneMovementHigh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float DeadZoneAimLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		float DeadZoneAimHigh;

	UPROPERTY(BlueprintAssignable, Category = "Player Functions || Movement")
		FChangeDirectionSignature OnChangedDirections;

	//The vertical movement speed without deadzones applied
	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetVerticalMovementRaw() { return VerticalMovementSpeedRaw; };

	//The vertical movement speed after deadzones are applied.
	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetVerticalMovement() { return VerticalMovementSpeed; };

	//The horizontal movement speed without any deadzones applied
	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetHorizontalMovementRaw() { return HorizontalMovementSpeedRaw; };

	//The horizontal movement speed after deadzones are applied.
	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetHorizontalMovement() { return HorizontalMovementSpeed; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetVerticalAimRaw() { return VerticalAimSpeedRaw; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetVerticalAim() { return VerticalAimSpeed; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetHorizontalAimRaw() { return HorizontalAimSpeedRaw; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		float GetHorizontalAim() { return HorizontalAimSpeed; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Movement")
		EPlayerMovementDirection GetLastKnownDirection() { return LastKnownDirection; };

	//--------------------------------------------------------------------------------------------------------
	//  CONTEXT ACTION FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	bool bCanUseContextActions;

	UPROPERTY()
		TArray<FContextActionData> ContextActorArray;

	void AddActorToContextArray(AActor* NewActor, FText HUDText = FText::GetEmpty(), FLinearColor Color = FLinearColor::White, bool bShowButton = true);

	//bFlushAllActors; if set to true, we will clear the array and all the HUD text associated with it. Used when we activate a context action by button press.
	void RemoveActorFromContextArray(AActor* ActorToRemove, bool bFlushAllActors = false);

	//AI controllers are way more precise with pathfinding, so during pathfinding we will switch to the controller instead. 
	class AController* ContextController;

	//so we can keep track of when the character is AI or not, so we can properly take care of certain variables
	bool bIsContextAI = false;

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Context Actions")
		class AController* StartContextController();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Context Actions")
		void StopContextController(EPlayerMovementDirection NewDirection = EPlayerMovementDirection::MD_NONE);

	UFUNCTION(BlueprintPure, Category = "Player Functions || Context Actions")
		bool IsUsingAI() { return bIsContextAI; };

	UFUNCTION(BlueprintPure, Category = "Player Functions || Context Actions")
		bool PlayerIsOnFloor();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Context Actions")
		void DisableContextActions();

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Context Actions")
		void EnableContextActions();

	//--------------------------------------------------------------------------------------------------------
	//  UI FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	class APlayerHUD* PlayerHUD;

	UFUNCTION(BlueprintCallable, Category = "Morgan UI")
		class APlayerHUD* GetPlayerHUD();

	bool bElementMenuIsActive = false;

	bool bToolMenuIsActive = false;

	//--------------------------------------------------------------------------------------------------------
	// ELEMENT FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	TArray<FSaveDataPlayerElement> ElementsUnlockData;
	
	EPlayerElementType CurrentElement;

	UFUNCTION(BlueprintPure, Category = "Morgan Functions || Equipment")
		bool CanElementBeUsed(EPlayerElementType ElementType);

	UFUNCTION(BlueprintCallable, Category = "Morgan Functions || Equipment")
		void SetElementUnlocked(EPlayerElementType ElementType, bool bNewUnlock = true);

	UFUNCTION(BlueprintCallable, Category = "Morgan Functions || Equipment")
		void SetElementInactive(EPlayerElementType ElementType, bool bNewInactive);

	//--------------------------------------------------------------------------------------------------------
	// WEAPON FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Weapon")
		FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Weapon")
		TSubclassOf<class UPlayerWeaponBase> SwordTemplate;

	class UPlayerWeaponBase* EquippedWeapon;

	UFUNCTION()
		void RegisterWeaponComponent(class UPlayerWeaponBase* NewWeapon);

	UFUNCTION(BlueprintPure, Category = "Player Functions || Equipment")
		UPlayerWeaponBase* GetEquippedWeapon() { return EquippedWeapon; };

	//--------------------------------------------------------------------------------------------------------
	// TOOLS
	//--------------------------------------------------------------------------------------------------------

	TArray<class UPlayerToolBase*> Tools;

	TArray<FSaveDataPlayerTool> ToolsUnlockData;

	EPlayerToolType CurrentToolType; //this is primarily used for save data.

	UFUNCTION(BlueprintPure, Category = "Equipment || Tools")
		TArray<class UPlayerToolBase*> GetTools() { return Tools; };

	class UPlayerToolBase* EquippedTool;

	/** RegisterToolComponent is called from within the tool components themselves when they become active. **/
	void RegisterToolComponent(class UPlayerToolBase* NewTool);

	UFUNCTION(BlueprintPure, Category = "Player Functions || Equipment")
		class UPlayerToolBase* GetEquippedTool();

	UFUNCTION(BlueprintPure, Category = "Player Functions || Equipment")
		int32 GetToolIndex(TSubclassOf<UPlayerToolBase> ToolClass);
		//int32 GetToolIndex(TSubclassOf<UObject> ToolClass); //any reason why this was an UObject?

	UFUNCTION(BlueprintPure, Category = "Player Functions || Equipment")
		bool CanToolBeUsed(EPlayerToolType ToolToCheck);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetEquippedTool(EPlayerToolType ToolType);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void ForceEquippedTool(EPlayerToolType ToolType);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetToolUnlocked(EPlayerToolType ToolType, bool bNewUnlock = true);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetToolInactive(EPlayerToolType ToolType, bool bNewInactive);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetToolsAmmoInfinite(bool bInfinite);

	//--------------------------------------------------------------------------------------------------------
	// CLOTHING / ITEMS
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Clothing")
		TArray<class UDataAssetItem*> HeadWear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* EquippedHeadWear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Clothing")
		TArray<class UDataAssetItem*> Jackets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* EquippedJacket;

	//Try to merge the gloves and jackets together equipment wise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Clothing")
		TArray<class UDataAssetItem*> Gloves;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* EquippedGloves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Clothing")
		TArray<class UDataAssetItem*> Pants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* EquippedPants;

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetEquippedHeadWear(int32 Index); //replace later with something more easy to understand, like an ENUM, when we actually have different clothing

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetEquippedJacket(int32 Index); //replace later with something more easy to understand, like an ENUM, when we actually have different clothing

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetEquippedGloves(int32 Index); //replace later with something more easy to understand, like an ENUM, when we actually have different clothing

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Equipment")
		void SetEquippedBoots(int32 Index); //replace later with something more easy to understand, like an ENUM, when we actually have different clothing

		//Total item inventory (without equipment, tools or weapons). Use sorting in the HUD to separate the items.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment || Items")
		TArray<class UDataAssetItem*> Items;

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		void AddInventoryItem(class UDataAssetItem* NewItem);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		bool HasItemInInventory(class UDataAssetItem* ItemToCheck);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		void SetInventoryItemInactive(class UDataAssetItem* ItemToCheck, bool bNewInactive);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		void SetInventoryItemActive(class UDataAssetItem* ItemToCheck, bool bNewActive);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		void SetInventoryItemRollover(class UDataAssetItem* ItemToCheck, bool bNewRollover);

	UFUNCTION(BlueprintCallable, Category = "Player Functions || Items")
		void SetInventoryItemUnlocked(class UDataAssetItem* ItemToCheck, bool bNewUnlock);

	//--------------------------------------------------------------------------------------------------------
	// HEALTH FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	float MaxPlayerHealth;
	float CurrentPlayerHealth;

	UFUNCTION(BlueprintPure, Category = "Player Functions || Health")
		void GetPlayerHealth(float& MaxHealth, float& CurrentHealth) { MaxHealth = MaxPlayerHealth; CurrentHealth = CurrentPlayerHealth; };
};
