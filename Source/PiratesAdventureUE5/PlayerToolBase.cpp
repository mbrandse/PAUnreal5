// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerToolBase.h"
#include "PiratesAdventureUE5.h"
#include "PlayerCharacter.h"
#include "PiratesAdventureStaticLibrary.h"

UPlayerToolBase::UPlayerToolBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;

	//tool related
	ChargeState = EChargeState::CS_NONE;
	ToolUsageType = EToolUsageType::TUT_INSTANT;

	bToolIsActive = false;
	bToolHasAiming = false;
	bIsCoolingDown = false;
	bLockToolAiming = false;

	AimToolRight = 0.f;
	AimToolUp = 0.f;
	bToolIsAiming = false;
	bToolIsBeingUsed = false;

	AmmoCount = 1.f;
	bAmmoCountUpgraded = false;
	AmmoRechargeTime = 1.f;
	bAmmoRechargeTimeUpgraded = false;
	AmmoRechargeDelay = 1.0f;

	bShouldAlwaysTick = false;
	ChargeDelayTimeElapsed = 0.f;
}

//--------------------------------------------------------------------------------------------------------
// TOOL ACTIVATION AND REGISTRY
//--------------------------------------------------------------------------------------------------------

void UPlayerToolBase::InitializeComponent()
{
	ToolOwner = Cast<APlayerCharacter>(GetOwner());
	if (!ToolOwner)
	{
		UE_LOG(AnyLog, Error, TEXT("Couldn't register %s component with the player"), *(GetClass()->GetName()));
	}
	else
		ToolOwner->RegisterToolComponent(this);

	AmmoCurrent = bAmmoCountUpgraded ? AmmoCurrent : AmmoCurrent * 1.5f; //TODO: have a failsafe here that prevents getting decimal values.

	ReceiveToolRegister(ToolOwner);

	Super::InitializeComponent();
}

void UPlayerToolBase::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (ToolOwner
		&& ChargeState != EChargeState::CS_NONE)
	{
		if (ChargeState == EChargeState::CS_GODMODE)
		{
			float MaxAmmo = bAmmoCountUpgraded ? AmmoCurrent : AmmoCurrent * 1.5f; //we should have an easier system for this
			if (MaxAmmo != AmmoCurrent)
			{
				AmmoCurrent = MaxAmmo;
				//CALLBACK TO UI FOR COMPLETE ANIMATION
			}
		}
		else
		{
			switch (ChargeState)
			{
				case EChargeState::CS_DELAY:
					ChargeDelayTimeElapsed += DeltaTime;
					if (ChargeDelayTimeElapsed >= AmmoRechargeDelay)
					{
						ChargeDelayTimeElapsed = 0.f;
						ChargeState = EChargeState::CS_RECOVERCHARGE;
					}
					break;

				case EChargeState::CS_DEPLETECHARGE:
					break;

				case EChargeState::CS_RECOVERCHARGE:
					break;

				case EChargeState::CS_PENALTYCHARGE:
					break;
			}
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerToolBase::InitializeTool()
{
	if (ToolOwner)
	{
		//SetToolElement(ToolOwner->EquippedWeapon->CurrentElementType); //do we need to do this from the weapon? Why not from the character?
		SetToolElement(ToolOwner->CurrentElement);
	}

	ReceiveInitializeTool();
}

void UPlayerToolBase::DeinitializeTool()
{
	bLockToolAiming = false;

	StopTool();
	ReceiveDeinitializeTool();
}

//--------------------------------------------------------------------------------------------------------
// SETTING ELEMENT
//--------------------------------------------------------------------------------------------------------

void UPlayerToolBase::SetToolElement(EPlayerElementType NewElement)
{
	OnReceiveSetToolElement(NewElement);
}

EPlayerElementType UPlayerToolBase::GetToolElement()
{
	if (ToolOwner)
	{
		return ToolOwner->CurrentElement;
	}

	return EPlayerElementType::ELEMENT_NONE;
}

//--------------------------------------------------------------------------------------------------------
// TOOL OPERATION
//--------------------------------------------------------------------------------------------------------

bool UPlayerToolBase::CanStartTool()
{
	if (ToolUsageType == EToolUsageType::TUT_INSTANT)
	{
		return FMath::FloorToInt(AmmoCurrent) != 0 && !bIsCoolingDown;
	}
	else
	{
		return AmmoCurrent > 0.f && !bIsCoolingDown;
	}
}

void UPlayerToolBase::StartTool()
{
	UWorld* World = GetWorld();

	if (World)
	{
		if (!CanStartTool())
		{
			StartToolNoAmmo();
		}
		else
		{
			if (ToolUsageType == EToolUsageType::TUT_CHARGE)
			{
				//bToolChargeIsDecreasing = true;
				//AmmoCurrent = (float)FMath::FloorToInt(AmmoCurrent); //Make sure that we only use the fully recharged bits.
				//UE_LOG(AnyLog, Warning, TEXT("AMMOCURRENT AFTER TICK STARTS %f"), AmmoCurrent);

				if (!bShouldAlwaysTick && AmmoCurrent != 0.f)
				{
					SetComponentTickEnabled(true);
				}
			}
			else
			{
				if (!bShouldAlwaysTick)
					SetComponentTickEnabled(false); //don't recharge when using the tool. 

				AmmoCurrent -= 1.f;
				ChargeState = EChargeState::CS_DELAY;
			}

			ReceiveStartTool();

			bToolIsBeingUsed = true;
			World->GetTimerManager().SetTimer(ToolUsedHandle, this, &UPlayerToolBase::TurnOffToolBeingUsed, ToolBeingUsedTimer, false);
		}
	}
}

void UPlayerToolBase::TurnOffToolBeingUsed()
{
	bToolIsBeingUsed = false;
}

void UPlayerToolBase::StartToolNoAmmo()
{
	ReceiveStartToolNoAmmo();
}

void UPlayerToolBase::StopTool()
{
	if (GetWorld() && ChargeState == EChargeState::CS_DEPLETECHARGE)
	{
		if (ToolUsageType == EToolUsageType::TUT_CHARGE)
		{
			if (!bShouldAlwaysTick)
				SetComponentTickEnabled(false);
		}

		ChargeState = EChargeState::CS_DELAY;
	}

	ReceiveStopTool();
}

bool UPlayerToolBase::GetAmmoIsInfinite()
{		
	return ChargeState == EChargeState::CS_GODMODE;
}