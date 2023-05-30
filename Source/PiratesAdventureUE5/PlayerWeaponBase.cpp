// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWeaponBase.h"
#include "PlayerCharacter.h"
#include "PiratesAdventureStaticLibrary.h"
#include "PiratesAdventureUE5.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"

UPlayerWeaponBase::UPlayerWeaponBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//attack variables
	BaseDamage = 1.f;
	bIsAttacking = false;
	ComboCount = 0;
	MaxComboCount = 3;
	bCanRequestContinueCombo = false;
	ChargeTime = 0.5f;
	TimeSincePress = 0.f;
	bNextAttackIsSupport = false;
	bSupportHasBeenUsed = false;
	CurrentElement = EPlayerElementType::ELEMENT_DEFAULT; //we can assume the element is none by default.
	CurrentAttackType = EAttackType::AT_NULL; //we want an empty attack to begin with.
	
	//Cooldown base settings
	bAttackIsCoolingDown = false;
	CooldownTime = 0.5f;

	//Link the attack to WeaponAttack
	RamaMeleeWeapon_OnHit.AddDynamic(this, &UPlayerWeaponBase::WeaponOnHit);

	//Finally make sure the component gets initialized.
	bWantsInitializeComponent = true;
}

void UPlayerWeaponBase::InitializeComponent()
{
	WeaponOwner = Cast<APlayerCharacter>(GetOwner());
	if (!WeaponOwner)
	{
		UE_LOG(AnyLog, Error, TEXT("Couldn't register %s component with the player"), *(GetClass()->GetName()));
	}
	else
		WeaponOwner->RegisterWeaponComponent(this);

	Super::InitializeComponent();
}

//--------------------------------------------------------------------------------------------------------
// SET WEAPON ELEMENT
//--------------------------------------------------------------------------------------------------------

void UPlayerWeaponBase::SetWeaponElement(EPlayerElementType NewElement)
{
	CurrentElement = NewElement;
	ReceiveSetElement(CurrentElement);
}

void UPlayerWeaponBase::RemoveWeaponElement(EPlayerElementType ElementToRemove)
{
	ReceiveRemoveElement(ElementToRemove);
}

//--------------------------------------------------------------------------------------------------------
// ATTACK FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

void UPlayerWeaponBase::OnAttackPressed()
{
	//get the time since press, for charging.
	TimeSincePress = GetWorld()->GetRealTimeSeconds();
}

void UPlayerWeaponBase::OnAttackReleased(float HorizontalInput, float VerticalInput)
{
	if(WeaponOwner)
	{
		bool bNextAttackIsCharge;

		if(TimeSincePress - GetWorld()->GetRealTimeSeconds() <= ChargeTime)
			bNextAttackIsCharge = true;

		TimeSincePress = 0.f; //Always reset it, since we already have the required bool.

		//First get the row names. We need these no matter what anyway
		static const FString Context = FString(TEXT("Context")); //not sure why this is necessary; double check somewhere. 
		TArray<FName> AttackRowNames = AttackDataList->GetRowNames();

		//doublecheck if this works, and whether we are getting the correct pivot foot with this.
		//Get the pivot foot based on combo count
		EPivotFoot RequiredPivotFoot = static_cast<EPivotFoot>(ComboCount % 2);
		float SidewaysMoveSpeed = WeaponOwner->GetCapsuleComponent()->GetPhysicsLinearVelocity().Size2D();

		//Get the current required attack
		//first evaluate if we are jumping
		if (WeaponOwner->bIsJumping || WeaponOwner->bIsDoubleJumping)
		{
			//We can only have one jump attack at any given time, so if we are already attacking, we don't need to do anything.
			if(!bIsAttacking)
			{
				//Do a linetrace here, so we don't get any attacks that need animation time even if the character has already landed.
				FVector Start = WeaponOwner->GetActorLocation();
				Start.Z -= WeaponOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				FVector End = Start - (WeaponOwner->GetActorUpVector() * JumpAttackCutoffDistance);
				FHitResult Hit;

				bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);

				if(!bHit)
				{
					if (SidewaysMoveSpeed < JumpSpeedForChargeAttack)
					{
						CurrentAttackType = EAttackType::AT_COMBOJUMP;
					}
					else
						CurrentAttackType = EAttackType::AT_CHARGEJUMP;
				}
			}
		}
		else if(WeaponOwner->IsDucking())
		{
			CurrentAttackType = EAttackType::AT_COMBODUCK; //we will assume that if the player started an attack from a non-ducking pose, player cannot duck anymore.
		}
		else
		{
			//if it's not a jump or duck attack, we can assume it's a general weapon attack
			//First see if we are not attacking yet and player is running
			if (SidewaysMoveSpeed < MoveSpeedForRunCombo
				&& !bIsAttacking
				&& ComboCount == 0)
			{
				//In this case, ignore any upwards input. We are already moving forward anyhoo. 
				CurrentAttackType = EAttackType::AT_COMBORUN;
			}
			else
			{
				//Perform normal check. For now overhead gets priority. Here we also need to differentiate between charge and normal
				if (VerticalInput >= 0.7f)
				{
					CurrentAttackType = bNextAttackIsCharge ? EAttackType::AT_COMBOOVERHEAD : EAttackType::AT_CHARGESLASH;
				}
				else if (VerticalInput >= 0.7f)
				{
					CurrentAttackType = bNextAttackIsCharge ? EAttackType::AT_COMBOSTAB : EAttackType::AT_CHARGESTAB;
				}
				else
					CurrentAttackType = bNextAttackIsCharge ? EAttackType::AT_COMBOSLASH  : EAttackType::AT_CHARGESLASH;
			}
		}

		if(!bNextAttackIsCharge)
			ComboCount++; //raise combocount for next combo, but only if next attack is not a charge attack. 

		if(!bIsAttacking)
		{
			//play the first animation from this function directly. Rely on others to be called from the animation.
			bIsAttacking = true;
			PlayRelevantAttackAnimation(RequiredPivotFoot, CurrentAttackType);
		}
	}
}

//--------------------------------------------------------------------------------------------------------
// DAMAGE FUNCTIONALITY
//--------------------------------------------------------------------------------------------------------

void UPlayerWeaponBase::WeaponOnHit(class AActor* HitActor, class UPrimitiveComponent* HitComponent, const FVector& ImpactPoint, const FVector& ImpactNormal, int32 ShapeIndex, FName HitBoneName, const struct FHitResult& HitResult)
{
	if (HitActor != WeaponOwner
		&& HitComponent->GetAttachmentRootActor() != WeaponOwner
		&& CanHitActor(HitActor)
		&& CanHitComponent(HitComponent)
		&& RamaMeleeWeapon_OnHit.IsBound())
	{
		//weapons just hit. No animation here. 
		FHitResult Hit = HitResult; //since the bloody hit result is constant.
		Hit.BoneName = HitBoneName;

		//Make a separate function here that checks the physics of the object, since this will not actually apply damage to objects without physics. 
		//Test whether that is still true, actually. 
		UGameplayStatics::ApplyPointDamage(HitActor, BaseDamage, ImpactPoint * -1.f, Hit, WeaponOwner->GetController(), WeaponOwner, CurrentDamageType);
	}
}

bool UPlayerWeaponBase::CanHitActor(AActor* HitActor)
{
	int32 HitIndex = HitActors.Find(HitActor);
	if (HitIndex == INDEX_NONE)
	{
		HitActors.Add(HitActor);
		return true;
	}

	return false;
}

//Do we need this?
bool UPlayerWeaponBase::CanHitComponent(UPrimitiveComponent* HitComponent)
{
	int32 HitIndex = HitComponents.Find(HitComponent);
	if (HitIndex == INDEX_NONE)
	{
		HitComponents.Add(HitComponent);
		return true;
	}

	return false;
}

void UPlayerWeaponBase::StopSwingDamage()
{
	Super::StopSwingDamage();

	HitActors.Empty();
	HitComponents.Empty();
}