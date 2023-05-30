// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RamaMeleeWeapon.h"
#include "PlayerWeaponBase.generated.h"

/**
 * 
 */

struct EPlayerElementType;

/** The current attack type. Note sure whether we are gonna use this, but it could be usefull for loading the proper animation **/
//consider using the 5.1 enhanced input system to create your own custom trigger, with our custom input value as well.
UENUM()
enum class EAttackType : uint8
{
	AT_NULL UMETA(DisplayName = "None", ToolTip = "NULL/None is used when attack data cannot be found in the AttackLookupTable"),
	AT_COMBOSLASH UMETA(DisplayName = "Combo Slash", ToolTip = "A normal attack that cannot be chained into a charge attack"),
	AT_COMBORUN UMETA(DisplayName = "Combo Run", ToolTip = "A normal attack from a run state, which will replace only the first combo in the sequence"),
	AT_COMBOOVERHEAD UMETA(DisplayName = "Combo Overhead", ToolTip = "A normal attack that cannot be chained into a charge attack"),
	AT_COMBOSTAB UMETA(DisplayName = "Combo Stab", ToolTip = "A normal attack that cannot be chained into a charge attack"),
	AT_COMBODUCK UMETA(DisplayName = "Combo Duck", ToolTip = "A normal attack that cannot be chained into a charge attack"),
	AT_COMBOJUMP UMETA(DisplayName = "Combo Jump", ToolTip = "A normal attack that cannot be chained into a charge attack"),
	AT_CHARGESLASH UMETA(DisplayName = "Charge Slash", ToolTip = "A charge attack chained from a regular/overhead slash attack"),
	AT_CHARGESTAB UMETA(DisplayName = "Charge Stab", ToolTip = "A charge attack chained from a stab attack"),
	AT_CHARGEJUMP UMETA(DisplayName = "Charge Jump", ToolTip = "The impact from a high speed jump attack (technically not charged)")
};

/** The current supporting foot of the animation. We need this to properly alternate between all attacks**/
UENUM()
enum class EPivotFoot : uint8
{
	PF_RIGHT UMETA(DisplayName = "Pivot Foot Right"), //We set the first element to right, so we can technically use this ENUM in combination with %
	PF_LEFT UMETA(DisplayName = "Pivot Foot Left")
};

USTRUCT(BlueprintType)
struct FAttackLookupTable : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	//Anim Montages for the attack. We use arrays here for random choosing of weapons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
		TArray<class UAnimMontage*> Attacks;
	
	//From when we should play the montage. We should allow some extra frames for blending. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
		float AttackStartTime;

	//Foot that serves as the main weight bearer for the attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
		EPivotFoot AttackPivot;

	//Type of attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
		EAttackType AttackType;
};

UCLASS(ClassGroup = Weapons, meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class PIRATESADVENTUREUE5_API UPlayerWeaponBase : public URamaMeleeWeapon
{
	GENERATED_BODY()

	public:

	UPlayerWeaponBase(const FObjectInitializer& ObjectInitializer);

	//FUNCTIONALITY FOR REGISTERING AND INITIALIZING WEAPON
	virtual void InitializeComponent() override;

	//Do we need this? 
	//void RegisterComponent();

	class APlayerCharacter* WeaponOwner;

	//--------------------------------------------------------------------------------------------------------
	// REGISTER AND EQUIP FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Weapon", meta = (DisplayName = "On Weapon Register"))
		void ReceiveWeaponRegister(class APlayerCharacter* Owner);

	UFUNCTION()
		virtual void OnEquipWeapon();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Weapon", meta = (DisplayName = "On Weapon Equip"))
		void ReceiveOnEquipWeapon();

	UFUNCTION()
		virtual void OnUnequipWeapon();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Weapon", meta = (DisplayName = "On Weapon Unequip"))
		void ReceiveOnUnequipWeapon();

	//--------------------------------------------------------------------------------------------------------
	// ELEMENT SWITCH FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Element")
		void SetWeaponElement(EPlayerElementType NewElement);

	UFUNCTION(BlueprintCallable, Category = "Element")
		void RemoveWeaponElement(EPlayerElementType ElementToRemove);

	//Much better than the old implementation, where we had functions for every single element. 
	UFUNCTION(BlueprintImplementableEvent, Category = "Element", meta = (DisplayName = "Set Weapon Element", ExpandEnumAsExecs = "NewElement"))
		void ReceiveSetElement(EPlayerElementType NewElement);

	UFUNCTION(BlueprintImplementableEvent, Category = "Element", meta = (DisplayName = "Remove Weapon Element", ExpandEnumAsExecs = "ElementToRemove"))
		void ReceiveRemoveElement(EPlayerElementType ElementToRemove);

	UPROPERTY(BlueprintReadOnly, Category = "Element")
		EPlayerElementType CurrentElement;

	//--------------------------------------------------------------------------------------------------------
	// ATTACK FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		class UDataTable* AttackDataList;

	//Special animation to take care of character getting up after landing from a jump charge.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
		class UAnimMontage* LandAfterJumpChargeMontage;

	//Required for various states; some functionality should be ignored when the player is attacking
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
		bool bIsAttacking;

	int32 ComboCount = 0;
	bool bCanRequestContinueCombo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attack")
		int32 MaxComboCount;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attack")
		float ChargeTime;

	//For now, let's only check cutoff distance. If necessary, let's check the speed as well.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attack")
		float JumpAttackCutoffDistance;

	//The minimum speed required to make the judgement between normal and charge for jump attacks, since we are not going to have charge attacks for jumps in the traditional sense.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attack")
		float JumpSpeedForChargeAttack;

	//The minimum speed to morph the first combo into a run combo. This is basically a stab attack with a bit more range, since we need to reduce speed to 0. 
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attack")
		float MoveSpeedForRunCombo;

	//Start the attack sequence
	void OnAttackPressed(); //attack pressed just initiates the count; the actual functionality will be done through the released function. 
	void OnAttackReleased(float HorizontalInput, float VerticalInput); //cues the actual attack (or starts it if it's the first attack in the chain)
	void OnActivateSupport();

	//This one needs to be called from the animation, to continue the combo (or do nothing when not). It will call the PlayRelevantAnimation. 
	void ProceedToNextCombo();

	//We need this to determine whether we can initiate a charge attack or not.
	float TimeSincePress;

	//Check whether the next attack is a charge. Don't include jump, since that uses a different pathway.
	bool NextAttackIsCharge() {
		return CurrentAttackType == EAttackType::AT_CHARGESLASH || CurrentAttackType == EAttackType::AT_CHARGESLASH;
	}
	//TODO:setup support attack in the tool; we don't want the sword to call it due to ammo count and whatnot.
	bool bNextAttackIsSupport; //Players can use one support attack per chain, and will not raise the combo counter. 
	bool bSupportHasBeenUsed;

	//This function requests the playing of the relevant attack montage. 
	UFUNCTION(BlueprintCallable, Category = "Attack")
		void PlayRelevantAttackAnimation(EPivotFoot PivotFoot, EAttackType AttackType);

	//Some attacks need to be properly followed up, so save the current attack type (current example; jump charge attack)
	EAttackType CurrentAttackType;

	//Finalize the attack sequence, (call from animation directly to stop combo if no button was pressed, or call on final combo). 
	void StopAttack();

	//When landed, we may need feedback for attacks
	void OnAttackLanded(FHitResult Hit);

	//An override, so we can clear out our own arrays. 
	void StopSwingDamage(); 

	TArray<AActor*> HitActors;
	TArray<UPrimitiveComponent*> HitComponents;

	bool CanHitActor(AActor* HitActor);
	bool CanHitComponent(UPrimitiveComponent* HitComponent);

	//Our main attack function
	UFUNCTION()
		virtual void WeaponOnHit(
			class AActor* HitActor,
			class UPrimitiveComponent* HitComponent,
			const FVector& ImpactPoint,
			const FVector& ImpactNormal,
			int32 ShapeIndex,
			FName HitBoneName,
			const struct FHitResult& HitResult);

	//ImpactSize is calculated from ImpactPoint (in local space), using ImpactPoint as bottomcenter pivot.  
	UFUNCTION()
		void WeaponOnChargeHit(
			FVector ImpactPoint,
			FVector ImpactHalfSize,
			bool bIncludePlayerInRange = false
		)


	//--------------------------------------------------------------------------------------------------------
	// DAMAGE DATA
	//--------------------------------------------------------------------------------------------------------

	//Combo and charge attack is equal to base damage, but can be boosted through elemental weaknesses and enemy states (stunned enemies will receive extra damage)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		float BaseDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> CurrentDamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageTypeDefault;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageTypeFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageTypeIce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageTypeWind;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		TSubclassOf<class UDamageType> DamageTypeWood;	

	//--------------------------------------------------------------------------------------------------------
	// ATTACK COOLDOWN
	//--------------------------------------------------------------------------------------------------------

	virtual void StartAttackCooldown();

	virtual void StopAttackCooldown();

	virtual void ResetAttackCooldown();

	UPROPERTY()
		FTimerHandle CooldownHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Cooldown")
		float CooldownTime;

	UPROPERTY()
		bool bAttackIsCoolingDown;
};
