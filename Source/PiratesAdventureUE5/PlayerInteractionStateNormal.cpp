// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInteractionStateNormal.h"
#include "PlayerCharacter.h"
#include "PlayerWeaponBase.h"
#include "PlayerToolBase.h"

UPlayerInteractionStateNormal::UPlayerInteractionStateNormal()
{
	StateID = EPlayerStateType::GPS_GameNormal;
	bIsDefaultState = true;
	bIsDucking = false;
	RotationRate = 500.f;
	DeadzoneNormalLow = 0.25f;
	DeadzoneNormalHigh = 0.9f;
	DeadzoneCrouchDegrees = 20.f;
	CrouchEnterRange = -0.8f;
	CrouchMaxInterval = 0.05f;
	CrouchEnterZone = 22.5;
	CrouchExitZone = 50.f;
	DodgeCooldown = 1.f;
	bCanDodge = true;
	bIsDefending = false;

	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	MoveMode = EMoveMode::MD_WALKING;
}

//--------------------------------------------------------------------------------------------------------
// DEFAULT FUNCTIONS
//--------------------------------------------------------------------------------------------------------

// Called when the game starts
void UPlayerInteractionStateNormal::InitializeComponent()
{
	Super::InitializeComponent();
}

// Called every frame
void UPlayerInteractionStateNormal::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//--------------------------------------------------------------------------------------------------------
// STATE FUNCTIONS
//--------------------------------------------------------------------------------------------------------

void UPlayerInteractionStateNormal::BeginState()
{
	Super::BeginState();
}

void UPlayerInteractionStateNormal::EndState(EPlayerStateType NewState)
{
	if (StateOwner->bIsJumping || StateOwner->bIsDoubleJumping)
	{
		StateOwner->bIsJumping = false;
		StateOwner->bIsDoubleJumping = false;
	}

	MoveMode = EMoveMode::MD_WALKING;

	Super::EndState(NewState);
}

//--------------------------------------------------------------------------------------------------------
// JUMP FUNCTIONS
//--------------------------------------------------------------------------------------------------------

//JUMPING FUNCTIONALITY
void UPlayerInteractionStateNormal::PerformJump()
{
	if (StateOwner->Controller != NULL)
	{
		//No jumps during crouch/duck/attack/aim
		if (IsDefending()
			|| StateOwner->GetMovementDisabled()
			|| StateOwner->bIsCrouched
			|| StateOwner->bIsDucked)
			return;

		if (StateOwner->EquippedWeapon
			&& StateOwner->EquippedWeapon->bIsAttacking)
			return;

		if (StateOwner->EquippedTool
			&& StateOwner->EquippedTool->bToolHasAiming
			&& StateOwner->EquippedTool->bToolIsActive)
			return;

		if (!StateOwner->bIsJumping)
		{
			StateOwner->bIsJumping = true;
			StateOwner->Jump();
			return;
		}
		else if (!StateOwner->bIsDoubleJumping && StateOwner->bCanDoubleJump && StateOwner->bIsJumping)
		{
			StateOwner->DoubleJump();
			return;
		}
	}
}

void UPlayerInteractionStateNormal::Landed(const FHitResult& Hit)
{
	if (StateOwner)
	{
		if (StateOwner->EquippedWeapon)
			StateOwner->EquippedWeapon->LandFromAttack();
	}
}