// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInteractionStateBase.h"
#include "PiratesAdventureUE5.h"
#include "PlayerCharacter.h"

// Sets default values for this component's properties
UPlayerInteractionStateBase::UPlayerInteractionStateBase()
{
	PrimaryComponentTick.bCanEverTick = true;

	bWantsInitializeComponent = true;
	StateID = EPlayerStateType::GPS_None;
	bIsDefaultState = false;
	bResetState = false;
	StateCooldownDuration = 0.f;
	bCanUseContextActions = false;
	bContextActionsArePaused = false;
}

// Called when the game starts
void UPlayerInteractionStateBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UPlayerInteractionStateBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerInteractionStateBase::InitializeComponent()
{
	Super::InitializeComponent();

	SetOwner(Cast<APlayerCharacter>(GetOwner()));
	if (!StateOwner)
	{
		UE_LOG(AnyLog, Error, TEXT("Couldn't register %s component with the player"), *(GetClass()->GetName()));
	}
	else
		StateOwner->RegisterStateComponent(this);
}

//"Closes" the state every time the state is switched and is responsible for the actual switching.
void UPlayerInteractionStateBase::EndState(EPlayerStateType NewState)
{
	StateOwner->SwitchPlayerState(NewState);

	if (StateCooldownDuration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(StateCooldownHandle, this, &UPlayerInteractionStateBase::StopStateCooldown, StateCooldownDuration, false);
	}
}

bool UPlayerInteractionStateBase::CanEnterState()
{
	if (StateCooldownDuration <= 0.f)
		return true;

	return !GetWorld()->GetTimerManager().IsTimerActive(StateCooldownHandle);
}

void UPlayerInteractionStateBase::EnableContext()
{
	if(StateOwner)
	{
		//Call the overlap event on all overlapping actors, so we can force any context actions if we happen to overlap it while enabling this.
		TArray<UPrimitiveComponent*> OverlappingComponents;
		StateOwner->GetCapsuleComponent()->GetOverlappingComponents(OverlappingComponents);

		if (OverlappingComponents.Num() != 0)
		{
			for (UPrimitiveComponent* Comp : OverlappingComponents)
			{
				if (Comp && Comp->OnComponentBeginOverlap.IsBound())
				{
					FHitResult HitResult;;

					HitResult = FHitResult(StateOwner, StateOwner->GetCapsuleComponent(), StateOwner->GetActorLocation(), FVector::ZeroVector);
					FOverlapInfo NewInfo = FOverlapInfo(HitResult);

					Comp->BeginComponentOverlap(NewInfo, true);
				}
			}
		}
	}

	bContextActionsArePaused = false;
}

void UPlayerInteractionStateBase::DisableContext()
{
	if(StateOwner)
		StateOwner->RemoveActorFromContextArray(NULL, true); //flush all context actors just in case. Its not like we can update it anyway.

	bContextActionsArePaused = true;
}