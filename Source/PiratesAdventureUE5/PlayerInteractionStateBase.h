// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCharacter.h"
#include "PlayerInteractionStateBase.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PIRATESADVENTUREUE5_API UPlayerInteractionStateBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInteractionStateBase();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Necessary for our own initializations
	virtual void InitializeComponent() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//--------------------------------------------------------------------------------------------------------
	// STATE FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	class APlayerCharacter* StateOwner;
	EPlayerStateType StateID;
	bool bIsDefaultState;
	bool bResetState; //Reset State is required to drive the animation blueprint, so we can reset a state that has clear beginnings and endings.
	bool bCanUseContextActions;
	bool bContextActionsArePaused;
	FTimerHandle StateCooldownHandle;

	UPROPERTY(BlueprintReadOnly)
		float StateCooldownDuration;

	// Called every time the game switches to this state, to "open" the state.
	virtual void BeginState() {};

	// Called when the game wants to switch to a different state, to "close" the state. 
	virtual void EndState(EPlayerStateType NewState);

	void SetOwner(class APlayerCharacter* NewOwner) { StateOwner = NewOwner; };

	UFUNCTION(BlueprintPure, Category = "Player State")
		bool CanResetState() { return bResetState; };

	virtual void StopStateCooldown() {};
	virtual bool CanEnterState();

	UFUNCTION(BlueprintPure, Category = "Player State")
		bool StateCanUseContextActions() { return bCanUseContextActions && !bContextActionsArePaused; };

	//--------------------------------------------------------------------------------------------------------
	// STATE INPUT FUNCTIONALITY
	//--------------------------------------------------------------------------------------------------------

	virtual void MoveRightLeft(float Value) {};
	virtual void MoveUpDown(float Value) {};

	virtual void LookRightLeft(float Value) {};
	virtual void LookUpDown(float Value) {};

	virtual void PerformJump() {};
	virtual void Landed(const FHitResult& Hit) {};

	virtual void StartAttack() {};
	virtual void StopAttack() {};

	virtual void StartDefend() {};
	virtual void StopDefend() {};

	//virtual void StartDodge() {}; //Will now be handled through Start/StopDefend

	virtual void StartContext() {};
	virtual void StopContext() {};
	virtual void EnableContext();
	virtual void DisableContext();

	class AActor* ActiveContextActor; //we need this so we stop the correct context action

	virtual void StartTool() {};
	virtual void StopTool() {};	

	virtual void UIUp() {};
	virtual void UILeft() {};
	virtual void UIRight() {};
	virtual void UIDown() {};
};
