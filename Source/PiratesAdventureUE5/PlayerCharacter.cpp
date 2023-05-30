// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "PlayerMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerSpringArmComponent.h"
#include "CineCameraComponent.h"
#include "PlayerInteractionStateBase.h"
#include "PiratesAdventureFunctionLibrary.h"
#include "PlayerHUD.h"
#include "PlayerWeaponBase.h"
#include "PlayerToolBase.h"
#include "PiratesAdventureStaticLibrary.h"
#include "DataAssetItem.h"
#include "RamaSaveComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCapsuleComponent()->SetHiddenInGame(true);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true); //do we need this?

	//for now lets skip the frontal hit collision components that we used in the original game. If they turn out to be necessary, get them then.

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Create a save system component for the character, and set up the inventory variables for saving.
	SaveSystemComponent = CreateDefaultSubobject<URamaSaveComponent>(TEXT("SaveSystemComponent"));
	SaveSystemComponent->RamaSave_PersistentActorUniqueID = FGuid::NewGuid(); //We need a GUID for this character, otherwise we cannot stop it from teleporting without our input
	SaveSystemComponent->RamaSave_ShouldLoadActorWorldPosition = false; //this will still load the cam pos, but since our cam is fixed anyway, that's fine. Keep in mind we cannot go from crawl to walk state.
	SaveSystemComponent->RamaSaveEvent_ActorFullyLoaded.AddDynamic(this, &APlayerCharacter::PlayerFullyLoaded);
	//Data to save (add later if necessary)
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("CurrentElement"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("ElementsUnlockData"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("CurrentToolType"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("ToolsUnlockData"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("MaxPlayerHealth"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("CurrentPlayerHealth"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("bEnteredFromDifferentMap"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("TargetPlayerStart"));
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("SavedLocationData")); //we will use this to set player in case we didn't enter from a different level
	SaveSystemComponent->RamaSave_OwningActorVarsToSave.Add(TEXT("SavedDirectionData"));
	//We may need to save direction data too, as that one determines the rotation. Really dont want to throw too much data into the 

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<UPlayerSpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 1400;
	CameraBoom->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f).Quaternion());

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCineCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera
	SideViewCameraComponent->LensSettings.MinFStop = 0.01f;
	SideViewCameraComponent->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
	SideViewCameraComponent->FocusSettings.ManualFocusDistance = 1400.f;
	SideViewCameraComponent->CurrentFocalLength = 18.f; //18.f corresponds to a 90 degrees FOV, which is what we have used so far for the game camera.
	SideViewCameraComponent->CurrentAperture = 0.05f; //0.05f gives us a somewhat decent DOP. It's an unrealistic Aperture, but who cares. 

	//Movement variables
	bIsDucked = false;
	bCanDoubleJump = true;
	bIsJumping = false;
	bIsDoubleJumping = false;
	JumpSingleHeight = 1200.f;
	JumpDoubleHeight = 800.f;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->GravityScale = 2.8f;
	GetCharacterMovement()->AirControl = 2.0f;
	GetCharacterMovement()->JumpZVelocity = JumpSingleHeight;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
	GetCharacterMovement()->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::X);
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	//for special effects
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(250);

	//Health
	MaxPlayerHealth = 5.f;
	CurrentPlayerHealth = MaxPlayerHealth;

	//Save Data MOVE TO STATIC
	SavedLocationData = FVector::ZeroVector;
	SavedDirectionData = EPlayerMovementDirection::MD_NONE;

	//lets leave the clothing meshes for now
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerCharacter::PlayerFullyLoaded(class URamaSaveComponent* RamaSaveComponent, FString LevelPackageName)
{
	//Force equip the element/tools here. We should have the data according to Rama's component
	//bEnteredFromDifferentMap should be processed in the gamemode, since that one also determines what playerstart we need
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Face buttons
	PlayerInputComponent->BindAction("FaceDown", IE_Pressed, this, &APlayerCharacter::PerformJump);
	PlayerInputComponent->BindAction("FaceDown", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FaceRight", IE_Pressed, this, &APlayerCharacter::StartAttack);
	PlayerInputComponent->BindAction("FaceRight", IE_Released, this, &APlayerCharacter::StopAttack);
	//PlayerInputComponent->BindAction("FaceUp", IE_Pressed, this, ); //no bind for up yet
	//PlayerInputComponent->BindAction("FaceUp", IE_Released, this, ); //no bind for up yet
	PlayerInputComponent->BindAction("FaceLeft", IE_Pressed, this, &APlayerCharacter::StartContext);
	PlayerInputComponent->BindAction("FaceLeft", IE_Released, this, &APlayerCharacter::StopContext);

	//Shoulders/triggers
	PlayerInputComponent->BindAction("ShoulderLeft", IE_Pressed, this, &APlayerCharacter::StartDefend);
	PlayerInputComponent->BindAction("ShoulderLeft", IE_Released, this, &APlayerCharacter::StopDefend);
	PlayerInputComponent->BindAction("ShoulderRight", IE_Pressed, this, &APlayerCharacter::StartTool); //tool is a toggle based thing; don't need the released state
	PlayerInputComponent->BindAction("TriggerLeft", IE_Pressed, this, &APlayerCharacter::StartToolMenu);
	PlayerInputComponent->BindAction("TriggerLeft", IE_Released, this, &APlayerCharacter::StopToolMenu);
	PlayerInputComponent->BindAction("TriggerRight", IE_Pressed, this, &APlayerCharacter::StartElementMenu);
	PlayerInputComponent->BindAction("TriggerRight", IE_Released, this, &APlayerCharacter::StopElementMenu);

	//Cross buttons: don't have released states
	PlayerInputComponent->BindAction("CrossUp", IE_Pressed, this, &APlayerCharacter::CrossUp);
	PlayerInputComponent->BindAction("CrossLeft", IE_Pressed, this, &APlayerCharacter::CrossLeft);
	PlayerInputComponent->BindAction("CrossRight", IE_Pressed, this, &APlayerCharacter::CrossRight);
	PlayerInputComponent->BindAction("CrossDown", IE_Pressed, this, &APlayerCharacter::CrossDown);

	//UI functions
	PlayerInputComponent->BindAction("Start", IE_Pressed, this, &APlayerCharacter::ToggleStartMenu).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("UIUp", IE_Pressed, this, &APlayerCharacter::UIUp).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("UILeft", IE_Pressed, this, &APlayerCharacter::UILeft).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("UIDown", IE_Pressed, this, &APlayerCharacter::UIDown).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("UIRight", IE_Pressed, this, &APlayerCharacter::UIRight).bExecuteWhenPaused = true;

	//axis functions
	PlayerInputComponent->BindAxis("MoveRightLeft", this, &APlayerCharacter::MoveRightLeft);
	PlayerInputComponent->BindAxis("MoveUpDown", this, &APlayerCharacter::MoveUpDown);
	PlayerInputComponent->BindAxis("LookRightLeft", this, &APlayerCharacter::LookRightLeft);
	PlayerInputComponent->BindAxis("LookUpDown", this, &APlayerCharacter::LookUpDown);
}

//---------------------------------------------------------------------------------------------------------------
// HUD FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

APlayerHUD* APlayerCharacter::GetPlayerHUD()
{
	if (PlayerHUD)
	{
		return PlayerHUD;
	}
	else
	{
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
			return PlayerHUD;
		}
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------------------------
// STATE RELATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

void APlayerCharacter::RegisterStateComponent(UPlayerInteractionStateBase* NewState)
{
	if (NewState)
	{
		if (NewState->StateID != EPlayerStateType::GPS_None)
		{
			FPlayerStateController NewStateController;
			NewStateController.StateID = NewState->StateID;
			NewStateController.StateClass = NewState;

			RegisteredStates.Add(NewStateController);

			if (NewState->bIsDefaultState)
			{
				CurrentState = NewState;
				CurrentStateType = NewState->StateID;
			}

			//UE_LOG(AnyLog, Warning, TEXT("Has registered %s's state component"), *(RegisteredStates[RegisteredStates.Num() - 1].StateClass->GetName()));
		}
	}
}

UPlayerInteractionStateBase* APlayerCharacter::GetRegisteredState(EPlayerStateType State)
{
	if (RegisteredStates.Num() != 0)
	{
		for (int i = 0; i < RegisteredStates.Num(); i++)
		{
			if (State == RegisteredStates[i].StateID)
			{
				return RegisteredStates[i].StateClass;
			}
		}
	}

	return NULL;
}

void APlayerCharacter::SwitchPlayerState(EPlayerStateType NewState)
{
	if (GetRegisteredState(NewState))
	{
		for (int i = 0; i < RegisteredStates.Num(); i++)
		{
			if (NewState == RegisteredStates[i].StateID)
			{
				if (CurrentState == RegisteredStates[i].StateClass)
					return;
				else
				{
					CurrentState = RegisteredStates[i].StateClass;
					CurrentStateType = RegisteredStates[i].StateID;
					CurrentState->BeginState();
				}
			}
		}
	}
}

void APlayerCharacter::SetNewPlayerState(EPlayerStateType NewState, bool bForceStateReset)
{
	if (CurrentState->StateID != NewState)
	{
		CurrentState->EndState(NewState);
	}
	else if (bForceStateReset)
	{
		CurrentState->BeginState();
	}
}

//---------------------------------------------------------------------------------------------------------------
// ELEMENT RELATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

bool APlayerCharacter::CanElementBeUsed(EPlayerElementType ElementType)
{
	//TODO; WE NEED TO COMBINE THIS WITH THE SAVE SYSTEM, NOT THE OLD SYSTEM THAT ASKS DATA FROM WEIRD SOURCES. 
	//WE WANT TO MOVE THE UNLOCK DATA OUT OF THE ASSETS AND INTO THE SAVE SYSTEM
	//for (int32 Index = 0; Index < Items.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Weapon
	//		&& Items[Index]->ItemData.ElementType == ElementToCheck)
	//	{
	//		//return Items[Index]->bUnlocked;
	//		return true; //this basically means everything is unlocked, always. 
	//	}
	//}

	//New implementation
	for (FSaveDataPlayerElement Element : ElementsUnlockData)
	{
		if(Element.ElementType == ElementType)
			return Element.bUnlockedElement && !Element.bElementIsInactive;
	}

	return false;
}

void APlayerCharacter::SetElementUnlocked(EPlayerElementType ElementType, bool bNewUnlock /* = true */)
{
	//TODO; WE NEED TO COMBINE THIS WITH THE SAVE SYSTEM, NOT THE OLD SYSTEM THAT ASKS DATA FROM WEIRD SOURCES. 
	//WE WANT TO MOVE THE UNLOCK DATA OUT OF THE ASSETS AND INTO THE SAVE SYSTEM
	//for (int32 Index = 0; Index < Items.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Weapon
	//		&& Items[Index]->ItemData.ElementType == WeaponType
	//		&& Items[Index]->bUnlocked != bNewUnlock)
	//	{
	//		Items[Index]->bUnlocked = bNewUnlock;
	//		if (GetPlayerHUD())
	//			GetPlayerHUD()->SetRingUnlocked(Items[Index]);
	//	}
	//}

		//New implementation
	for (FSaveDataPlayerElement Element : ElementsUnlockData)
	{
		if (Element.ElementType == ElementType)
			Element.bUnlockedElement = bNewUnlock;
	}

	//TODO: ADD UI FEEDBACK CALLS HERE TOO!
}

void APlayerCharacter::SetElementInactive(EPlayerElementType ElementType, bool bNewInactive)
{
	//inactive states can remain here for now. We may need to save inactive states to the save system as well though,
	//considering that we may be able to save within inactive areas like underwater.
	//for (int32 Index = 0; Index < Items.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Weapon
	//		&& Items[Index]->ItemData.ElementType == WeaponType
	//		&& Items[Index]->bInactive != bNewInactive)
	//	{
	//		Items[Index]->bInactive = bNewInactive;
	//		if (GetPlayerHUD())
	//			GetPlayerHUD()->SetRingInactive(Items[Index]);
	//	}
	//}

	//New implementation
	for (FSaveDataPlayerElement Element : ElementsUnlockData)
	{
		if (Element.ElementType == ElementType)
			Element.bElementIsInactive = bNewInactive;
	}

	//TODO: ADD UI FEEDBACK CALLS HERE TOO!
}

//---------------------------------------------------------------------------------------------------------------
// WEAPON RELATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

void APlayerCharacter::RegisterWeaponComponent(UPlayerWeaponBase* NewWeapon)
{
	if (NewWeapon)
	{
		EquippedWeapon = NewWeapon;
		EquippedWeapon->SetRenderCustomDepth(true);
		EquippedWeapon->SetCustomDepthStencilValue(GetMesh()->CustomDepthStencilValue);
		EquippedWeapon->SetHiddenInGame(false);
		EquippedWeapon->RegisterComponent();
	}
}

//---------------------------------------------------------------------------------------------------------------
//	TOOL RELATED FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

//This should happen before the game even starts.
void APlayerCharacter::RegisterToolComponent(class UPlayerToolBase* NewTool)
{
	if (NewTool)
	{
		int32 ToolIndex = 0;
		Tools.Find(NewTool, ToolIndex);
		if (ToolIndex == -1 && NewTool->ToolType != EPlayerToolType::TOOL_NONE)
		{
			Tools.Add(NewTool);

			//UE_LOG(AnyLog, Warning, TEXT("Has registered %s's tool component"), *(Tools[Tools.Num() - 1]->GetName()));
		}
	}
}

UPlayerToolBase* APlayerCharacter::GetEquippedTool()
{
	return EquippedTool;
}

int32 APlayerCharacter::GetToolIndex(TSubclassOf<UPlayerToolBase> ToolClass)
{
	for (int32 Index = 0; Index < Tools.Num(); Index++)
	{
		if (Tools[Index]->GetClass() == ToolClass)
			return Index;
	}

	return -1;
}

void APlayerCharacter::ForceEquippedTool(EPlayerToolType ToolType)
{
	UWorld* World = GetWorld();

	if (World)
	{
		//TODO: Make sure the inventory part gets moved to the HUD, where all the item data is stored. 
		for (int32 ToolIndex = 0; ToolIndex < Tools.Num(); ToolIndex++)
		{
			if (Tools[ToolIndex]->ToolType == ToolType)
			{
				SetEquippedTool(ToolType);
			}
		}
	}
}

void APlayerCharacter::SetEquippedTool(EPlayerToolType ToolType)
{
	if (ToolType != EPlayerToolType::TOOL_NONE)
	{
		for (int32 Index = 0; Index < Tools.Num(); Index++)
		{
			if (Tools[Index] != NULL
				&& Tools[Index]->ToolType == ToolType
				&& CanToolBeUsed(ToolType))
			{
				//make sure to detach the previous weapon and set it to invisible, if we have one
				if (GetEquippedTool())
				{
					//maybe destroy the component?
					EquippedTool->DeinitializeTool();
					EquippedTool->SetHiddenInGame(true);
				}

				//Now initialize the new tool
				EquippedTool = Tools[Index];
				EquippedTool->SetHiddenInGame(true);
				EquippedTool->InitializeTool();
				EquippedTool->bToolIsActive = false;

				return;
			}
		}
	}
}

bool APlayerCharacter::CanToolBeUsed(EPlayerToolType ToolToCheck)
{
	//MOVE INTO SAVE DATA!!! INCLUDING INACTIVE!!!
	//for (int32 Index = 0; Index < Items.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Tool
	//		&& Items[Index]->ItemData.ToolType == ToolToCheck)
	//		return true;
	//		//return Items[Index]->bUnlocked && !Items[Index]->bInactive;
	//}

	//New implementation
	for (FSaveDataPlayerTool Tool : ToolsUnlockData)
	{
		if (Tool.ToolType == ToolToCheck)
			return Tool.bUnlockedTool && !Tool.bToolIsInactive;
	}

	return false;
}

void APlayerCharacter::SetToolUnlocked(EPlayerToolType ToolType, bool bNewUnlock)
{
	//USE SAVE DATA FOR THIS!!!
	//for (int32 Index = 0; Index < Tools.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Tool
	//		&& Items[Index]->ItemData.ToolType == ToolType
	//		&& Items[Index]->bUnlocked != bNewUnlock)
	//	{
	//		Items[Index]->bUnlocked = bNewUnlock;
	//		if (GetPlayerHUD() != NULL)
	//			GetPlayerHUD()->SetRingUnlocked(Items[Index]);
	//	}
	//}

	//New implementation
	for (FSaveDataPlayerTool Tool : ToolsUnlockData)
	{
		if (Tool.ToolType == ToolType)
			Tool.bUnlockedTool = bNewUnlock;
	}

	//TODO: ADD UI FEEDBACK CALLS HERE TOO!
}

void APlayerCharacter::SetToolInactive(EPlayerToolType ToolType, bool bNewInactive)
{
	//USE SAVE DATA FOR THIS!!!
	//for (int32 Index = 0; Index < Tools.Num(); Index++)
	//{
	//	if (Items[Index]->ItemData.Category == EItemCategory::IC_Tool
	//		&& Items[Index]->ItemData.ToolType == ToolType
	//		&& Items[Index]->bInactive != bNewInactive)
	//	{
	//		Items[Index]->bInactive = bNewInactive;
	//		//if (GetPlayerHUD() != NULL)
	//		//	GetPlayerHUD()->SetRingInactive(Items[Index]);
	//	}
	//}

	//New implementation
	for (FSaveDataPlayerTool Tool : ToolsUnlockData)
	{
		if (Tool.ToolType == ToolType)
			Tool.bToolIsInactive = bNewInactive;
	}

	//TODO: ADD UI FEEDBACK CALLS HERE TOO!
}

void APlayerCharacter::SetToolsAmmoInfinite(bool bInfinite)
{
	for (UPlayerToolBase* Tool : Tools)
	{
		if (bInfinite)
		{
			Tool->ChargeState = EChargeState::CS_GODMODE;
		}
		else
		{
			if (Tool->ChargeState == EChargeState::CS_GODMODE)
			{
				if (Tool == GetEquippedTool())
				{
					Tool->ChargeState = EChargeState::CS_DEPLETECHARGE;
				}
				else
					Tool->ChargeState = EChargeState::CS_NONE;
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------------------
// INVENTORY FUNCTIONS
//---------------------------------------------------------------------------------------------------------------

void APlayerCharacter::SetEquippedHeadWear(int32 Index)
{
	if (Index < 0)
	{
		EquippedHeadWear->SetVisibility(false);
	}
	else if (HeadWear.IsValidIndex(Index) && HeadWear[Index]->ItemData.MeshTemplate != NULL)
	{
		if (!EquippedHeadWear->IsVisible())
			EquippedHeadWear->SetVisibility(true);

		EquippedHeadWear->SetSkeletalMesh(HeadWear[Index]->ItemData.MeshTemplate);
	}
}

void APlayerCharacter::SetEquippedJacket(int32 Index)
{
	if (Index < 0)
	{
		EquippedJacket->SetVisibility(false);
	}
	else if (Jackets.IsValidIndex(Index) && Jackets[Index]->ItemData.MeshTemplate != NULL)
	{
		if (!EquippedJacket->IsVisible())
			EquippedJacket->SetVisibility(true);

		EquippedJacket->SetSkeletalMesh(Jackets[Index]->ItemData.MeshTemplate);
	}
}

void APlayerCharacter::SetEquippedGloves(int32 Index)
{
	if (Index < 0)
	{
		EquippedGloves->SetVisibility(false);
	}
	else if (Gloves.IsValidIndex(Index) && Gloves[Index]->ItemData.MeshTemplate != NULL)
	{
		if (!EquippedGloves->IsVisible())
			EquippedGloves->SetVisibility(true);

		EquippedGloves->SetSkeletalMesh(Gloves[Index]->ItemData.MeshTemplate);
	}
}

void APlayerCharacter::SetEquippedBoots(int32 Index)
{
	if (Index < 0)
	{
		EquippedPants->SetVisibility(false);
	}
	else if (Pants.IsValidIndex(Index) && Pants[Index]->ItemData.MeshTemplate != NULL)
	{
		if (!EquippedPants->IsVisible())
			EquippedPants->SetVisibility(true);

		EquippedPants->SetSkeletalMesh(Pants[Index]->ItemData.MeshTemplate);
	}
}

void APlayerCharacter::AddInventoryItem(class UDataAssetItem* NewItem)
{
	if (NewItem)
	{
		int32 Index;
		Items.Find(NewItem, Index);
		if (Index == INDEX_NONE)
		{
			Items.Add(NewItem);
		}
	}
}

bool APlayerCharacter::HasItemInInventory(class UDataAssetItem* ItemToCheck)
{
	if (ItemToCheck)
	{
		for (auto& Item : Items)
		{
			const UDataAssetItem* CheckedItem = CastChecked<UDataAssetItem>(Item);
			if (CheckedItem && CheckedItem == ItemToCheck)
				return true;
		}
	}

	return false;
}

void APlayerCharacter::SetInventoryItemActive(class UDataAssetItem* ItemToCheck, bool bNewActive)
{
	if (ItemToCheck)
	{
		for (UDataAssetItem* Item : Items)
		{
			if (Item == ItemToCheck)
				Item->bActive = bNewActive;
		}
	}
}

void APlayerCharacter::SetInventoryItemInactive(class UDataAssetItem* ItemToCheck, bool bNewInactive)
{
	if (ItemToCheck)
	{
		for (UDataAssetItem* Item : Items)
		{
			if (Item == ItemToCheck)
				Item->bInactive = bNewInactive;
		}
	}
}

void APlayerCharacter::SetInventoryItemRollover(class UDataAssetItem* ItemToCheck, bool bNewRollover)
{
	if (ItemToCheck)
	{
		for (UDataAssetItem* Item : Items)
		{
			if (Item == ItemToCheck)
				Item->bRollover = bNewRollover;
		}
	}
}

void APlayerCharacter::SetInventoryItemUnlocked(class UDataAssetItem* ItemToCheck, bool bNewUnlock)
{
	if (ItemToCheck)
	{
		//USE SAVE DATA FOR THIS!!! DO WE EVEN NEED AN UNLOCK FOR EVERY SINGLE ITEMASSET?
		//ITEMS THAT REMAIN VISIBLE AFTER AMMO HITS ZERO CAN BE SAVED JUST LIKE TOOLS. 
		for (UDataAssetItem* Item : Items)
		{
			//if (Item == ItemToCheck)
			//	Item->bUnlocked = bNewUnlock;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------
//	FACE ACTIONS
//---------------------------------------------------------------------------------------------------------------

void APlayerCharacter::PerformJump()
{
	if (CurrentState)
	{
		CurrentState->PerformJump();
	}
}

void APlayerCharacter::Jump()
{
	//Only jump when we are not already falling. Otherwise skip to DoubleJump right away. 
	if (GetVelocity().Z == 0.f)
	{
		Super::Jump();
	}
	else
	{
		bIsJumping = true;
		DoubleJump();
	}
}

void APlayerCharacter::DoubleJump()
{
	if (JumpDoubleHeight > 0)
	{
		bIsDoubleJumping = true;
		LaunchCharacter(FVector(0.f, 0.f, JumpDoubleHeight), false, true);
	}
}

void APlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bIsJumping = false;
	bIsDoubleJumping = false;

	if (CurrentState)
	{
		CurrentState->Landed(Hit);
	}

	if (OnPlayerLand.IsBound())
		OnPlayerLand.Broadcast(Hit);
}

void APlayerCharacter::StartAttack()
{
	if (CurrentState)
	{
		CurrentState->StartAttack();
	}
}

void APlayerCharacter::StopAttack()
{
	if (CurrentState)
	{
		CurrentState->StopAttack();
	}
}

//--------------------------------------------------------------------------------------------------------
//  CONTEXT ACTION FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

void APlayerCharacter::StartContext()
{
	if (CurrentState
		&& CurrentState->StateCanUseContextActions())
	{
		if (ContextActorArray.IsValidIndex(0)
			&& ContextActorArray[0].ContextActor != NULL)
		{
			IInterfaceContextAction* ContextAction = Cast<IInterfaceContextAction>(ContextActorArray[0].ContextActor);
			if (ContextAction)
			{
				CurrentState->ActiveContextActor = ContextActorArray[0].ContextActor;
				ContextAction->StartContextAction(this);
			}
		}

		CurrentState->StartContext();
	}
}

void APlayerCharacter::StopContext()
{
	if(CurrentState
		&& CurrentState->StateCanUseContextActions())
	{
		int32 ContextIndex = 0;
		if(CurrentState->ActiveContextActor)
		{
			for (int32 i = 0; i < ContextActorArray.Num(); i++)
			{
				if (ContextActorArray.IsValidIndex(i)
					&& ContextActorArray[i].ContextActor == CurrentState->ActiveContextActor)
				{
					ContextIndex = i;
					break;
				}
			}
		}

		//Do a double check just in case (do we really need this?)
		if (ContextActorArray.IsValidIndex(ContextIndex) 
			&& ContextActorArray[ContextIndex].ContextActor != NULL)
		{
			IInterfaceContextAction* ContextAction = Cast<IInterfaceContextAction>(ContextActorArray[0].ContextActor);
			if (ContextAction)
				ContextAction->StopContextAction(this);
		}

		CurrentState->ActiveContextActor = NULL;
		CurrentState->StopContext();
	}
}

void APlayerCharacter::AddActorToContextArray(AActor* NewActor, FText HUDText /* = FText::GetEmpty() */, FLinearColor Color /* = FLinearColor::White */, bool bShowButton /* = true */)
{
	//TODO: double check functionality here; sometimes context actions don't register correctly 
	//and pressing the button won't do anything then (even though the HUD is okay).
	if (NewActor
		&& CurrentState
		&& CurrentState->StateCanUseContextActions())
	{
		for (FContextActionData ActionData : ContextActorArray)
		{
			if (ActionData.ContextActor
				&& ActionData.ContextActor == NewActor) return;
		}

		FContextActionData NewContext;
		NewContext.ContextActor = NewActor;
		NewContext.ContextText = HUDText;
		NewContext.ContextColor = Color;
		NewContext.bShowButton = bShowButton;

		ContextActorArray.Insert(NewContext, 0);

		if (GetPlayerHUD() != NULL
			&& !HUDText.IsEmpty())
		{
			GetPlayerHUD()->SetContextText(HUDText, Color);
		}
	}
}

void APlayerCharacter::RemoveActorFromContextArray(AActor* ActorToRemove, bool bFlushAllActors /* = false */)
{
	if (GetPlayerHUD())
	{
		if (bFlushAllActors)
		{
			ContextActorArray.Empty();
			GetPlayerHUD()->ClearContextText();

			return;
		}

		UWorld* World = GetWorld();

		if (ActorToRemove
			&& World)
		{
			for (int32 Index = 0; Index < ContextActorArray.Num(); Index++)
			{
				if (ContextActorArray[Index].ContextActor == ActorToRemove)
				{
					ContextActorArray.RemoveAt(Index);

					//make sure to only clear text when the actor is actually the current active actor. Also refresh text to next action, if there are any.
					if (Index == 0)
					{
						GetPlayerHUD()->ClearContextText();
						if (ContextActorArray.Num() != 0)
							GetPlayerHUD()->SetContextText(ContextActorArray[0].ContextText, ContextActorArray[0].ContextColor);
					}
				}
			}
		}
	}
}

AController* APlayerCharacter::StartContextController()
{
	if (UWorld* World = GetWorld())
	{
		if (AIControllerClass != NULL
			&& ContextController == NULL)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.OverrideLevel = GetLevel();
			SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map
			ContextController = GetWorld()->SpawnActor<AController>(AIControllerClass, GetActorLocation(), GetActorRotation(), SpawnInfo);
		}

		if (ContextController != NULL)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				PC->bAutoManageActiveCameraTarget = false;
				PC->UnPossess();
				ContextController->Possess(this);
				PC->SetViewTargetWithBlend(this);

				DisableAxisLock();
				DisableAutoRotation();
				bIsContextAI = true;
			}
		}
	}

	return ContextController;
}

void APlayerCharacter::StopContextController(EPlayerMovementDirection NewDirection)
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC && ContextController)
		{
			ContextController->UnPossess();
			PC->Possess(this);
			PC->SetViewTargetWithBlend(this);
			PC->bAutoManageActiveCameraTarget = true;

			EnableAxisLock();

			if(NewDirection != EPlayerMovementDirection::MD_NONE)
				LastKnownDirection = NewDirection; //so we can make a custom rotation possible too.

			EnableAutoRotation();
			bIsContextAI = false;

			//Force a rerecord of the camera frustum, since it's not unlikely we have completely changed our Z position
			CameraBoom->RerecordCameraFrustum();
		}
	}
}

bool APlayerCharacter::PlayerIsOnFloor()
{
	if (GetCharacterMovement())
	{
		return GetCharacterMovement()->CurrentFloor.HitResult.GetActor() != NULL;
	}

	return true;
}

void APlayerCharacter::EnableContextActions()
{
	if (CurrentState)
		CurrentState->EnableContext();
}

void APlayerCharacter::DisableContextActions()
{
	if (CurrentState)
		CurrentState->DisableContext();
}

//--------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------
//  TRIGGER ACTIONS
//--------------------------------------------------------------------------------------------------------

void APlayerCharacter::StartDefend()
{
	if (CurrentState)
	{
		CurrentState->StartDefend();
	}
}

void APlayerCharacter::StopDefend()
{
	if (CurrentState)
	{
		CurrentState->StopDefend();
	}
}

void APlayerCharacter::StartTool()
{
	if (CurrentState)
	{
		CurrentState->StartTool();
	}
}

void APlayerCharacter::StartToolMenu()
{
	UWorld* World = GetWorld();

	if (World
		&& !bElementMenuIsActive
		&& GetPlayerHUD())
	{
		bToolMenuIsActive = true;
		GetPlayerHUD()->SwitchEquipVisualization(ERingMenuType::RMT_Tool);
	}
}

void APlayerCharacter::StopToolMenu()
{
	UWorld* World = GetWorld();

	if (World
		&& !bElementMenuIsActive
		&& GetPlayerHUD() != NULL)
	{
		bToolMenuIsActive = false;
		GetPlayerHUD()->ResetEquipVisualization();
	}
}

void APlayerCharacter::StartElementMenu()
{
	UWorld* World = GetWorld();

	if (World
		&& !bToolMenuIsActive
		&& GetPlayerHUD() != NULL)
	{
		bElementMenuIsActive = true;
		GetPlayerHUD()->SwitchEquipVisualization(ERingMenuType::RMT_Element);
	}
}

void APlayerCharacter::StopElementMenu()
{
	UWorld* World = GetWorld();

	if (World
		&& !bToolMenuIsActive
		&& GetPlayerHUD() != NULL)
	{
		bElementMenuIsActive = false;
		GetPlayerHUD()->ResetEquipVisualization();
	}
}

//--------------------------------------------------------------------------------------------------------
//  CROSS BUTTON ACTIONS
//--------------------------------------------------------------------------------------------------------

void APlayerCharacter::CrossUp()
{

}

void APlayerCharacter::CrossLeft()
{

}

void APlayerCharacter::CrossRight()
{

}

void APlayerCharacter::CrossDown()
{

}

//--------------------------------------------------------------------------------------------------------
//  UI FUNCTIONS
//--------------------------------------------------------------------------------------------------------

void APlayerCharacter::ToggleStartMenu()
{

}

void APlayerCharacter::UIUp()
{
	if(CurrentState)
		CurrentState->UIUp();
}

void APlayerCharacter::UILeft()
{
	if (CurrentState)
		CurrentState->UILeft();
}

void APlayerCharacter::UIRight()
{
	if (CurrentState)
		CurrentState->UIRight();
}

void APlayerCharacter::UIDown()
{
	if (CurrentState)
		CurrentState->UIDown();
}

//--------------------------------------------------------------------------------------------------------
//  AXIS AND MOVEMENT FUNCTIONS
//--------------------------------------------------------------------------------------------------------

void APlayerCharacter::MoveRightLeft(float Value)
{
	HorizontalMovementSpeedRaw = Value;

	//We will calculate the deadzone modified variables here, since we need both axes for this to work
	UPiratesAdventureFunctionLibrary::GetCorrectedInput(HorizontalMovementSpeedRaw, VerticalMovementSpeedRaw, DeadZoneMovementLow, DeadZoneMovementHigh, HorizontalMovementSpeed, VerticalMovementSpeed);

	//Set the movement direction
	if (Value != 0.f || LastKnownDirection == EPlayerMovementDirection::MD_NONE)
	{
		int32 CurrentDir = (int32)Value;
		EPlayerMovementDirection NewDir = EPlayerMovementDirection::MD_NONE;

		switch (CurrentDir)
		{
			case -1: NewDir = EPlayerMovementDirection::MD_LEFT;
				break;

			case 1: NewDir = EPlayerMovementDirection::MD_RIGHT;
				break;
		}

		if (NewDir != EPlayerMovementDirection::MD_NONE
			&& NewDir != LastKnownDirection
			&& OnChangedDirections.IsBound())
		{
			OnChangedDirections.Broadcast(NewDir);
		}

		LastKnownDirection = NewDir; //right now this implementation will force a set direction on every playerstart. Which is not good. 
	}

	if (CurrentState)
	{
		CurrentState->MoveRightLeft(Value);
	}
}

void APlayerCharacter::MoveUpDown(float Value)
{
	VerticalMovementSpeedRaw = Value;

	if (CurrentState)
	{
		CurrentState->MoveUpDown(Value);
	}
}

void APlayerCharacter::LookRightLeft(float Value)
{
	HorizontalAimSpeedRaw = Value;

	//We will calculate the deadzone modified variables here, since we need both axes for this to work
	UPiratesAdventureFunctionLibrary::GetCorrectedInput(HorizontalAimSpeedRaw, VerticalAimSpeedRaw, DeadZoneAimLow, DeadZoneAimHigh, HorizontalAimSpeed, VerticalAimSpeed);

	if (CurrentState)
	{
		CurrentState->LookRightLeft(Value);
	}
}

void APlayerCharacter::LookUpDown(float Value)
{
	VerticalAimSpeedRaw = Value;

	if (CurrentState)
	{
		CurrentState->LookUpDown(Value);
	}
}

void APlayerCharacter::ForceUnCrouch()
{
	UPlayerMovementComponent* PlayerMovement = Cast<UPlayerMovementComponent>(GetCharacterMovement());
	if (PlayerMovement)
	{
		PlayerMovement->UnDuck();
		PlayerMovement->UnCrouch();
	}
}

void APlayerCharacter::EnableAxisLock()
{
	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
}

void APlayerCharacter::DisableAxisLock()
{
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
}

void APlayerCharacter::DisableMovement()
{
	bDisableMovement = true;
}

void APlayerCharacter::EnableMovement()
{
	bDisableMovement = false;
}