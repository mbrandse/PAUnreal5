// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInteractionStateBase.h"
#include "PlayerInteractionStateNormal.generated.h"

/**
 * 
 */

//Used to determine what kind of movement we should process on moving left/right, used in C++ only
UENUM()
enum class EMoveMode : uint8
{
	MD_WALKING UMETA(DisplayName = "Walking"),
	MD_DUCKING UMETA(DisplayName = "Ducking"),
	MD_CROUCHING UMETA(DisplayName = "Crouching")
};

UCLASS(ClassGroup = (PlayerState), meta = (BlueprintSpawnableComponent, DisplayName = "Player State - Normal"))
class PIRATESADVENTUREUE5_API UPlayerInteractionStateNormal : public UPlayerInteractionStateBase
{
	GENERATED_BODY()

	public:

	UPlayerInteractionStateNormal();

	// Maybe we need it eventually. It's the default state after all.
	virtual void InitializeComponent() override;

	// Maybe we need it eventually. It's the default state after all.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//--------------------------------------------------------------------------------------------------------
	// STATE FUNCTIONS
	//--------------------------------------------------------------------------------------------------------

	virtual void BeginState();
	virtual void EndState(EPlayerStateType NewState);

	//--------------------------------------------------------------------------------------------------------
	// INPUT FUNCTIONS
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

	//unique defense related functions
	private:
	bool bIsDefending; 

	public:
	void ForceStopDefend();
	bool IsDefending() { return bIsDefending; }; 

	//dodge related functions, that we cannot stick into the defend options
	void StopDodge();
	
	FTimerHandle DodgeCooldownHandle;
	void StopDodgeCooldown();

	virtual void StartContext() {};
	virtual void StopContext() {};
	virtual void EnableContext();
	virtual void DisableContext();

	virtual void StartTool() {};
	virtual void StopTool() {};

	//We don't need these here in all likelyhood.
	//virtual void UIUp() {};
	//virtual void UILeft() {};
	//virtual void UIRight() {};
	//virtual void UIDown() {};

	//--------------------------------------------------------------------------------------------------------
	// DEADZONE DATA
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DeadzoneNormalLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DeadzoneNormalHigh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DeadzoneCrouchDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DeadzoneAimLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DeadzoneAimHigh;

	//--------------------------------------------------------------------------------------------------------
	// AUTO ROTATION RATE
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		FRotator DesiredRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float RotationRate;

	//--------------------------------------------------------------------------------------------------------
	// CROUCH AND DUCK DATA
	//--------------------------------------------------------------------------------------------------------

	//The minimum X input that is required to make a character start crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float CrouchEnterRange;

	//The maximum interval before we actually start crouching. Calculated in sec. Implemented so we don't accidentally hit down. Should be a small value, like 0.1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float CrouchMaxInterval;

	//The maximum angle at which the player can enter crouch mode, taken from the X and Y input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float CrouchEnterZone;

	//The maximum angle at which the player can exit crouch mode, taken from the X and Y input
	//Consider making this slightly higher than 45 degrees, since keyboard XY input becomes 45 degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float CrouchExitZone;

	bool bIsDucking;

	EMoveMode MoveMode;

	//Bool functions, so we can concentrate annoying "&&" conditionals into one place. 
	bool CanDuck();

	bool CanCrouch();

	//--------------------------------------------------------------------------------------------------------
	// DODGE DATA
	//--------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morgan Movement")
		float DodgeCooldown;

	bool bCanDodge;

	UPROPERTY()
		class UPlayerMovementComponent* MovementComponent;

	
};
