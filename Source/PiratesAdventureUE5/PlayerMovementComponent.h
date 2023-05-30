// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UPlayerMovementComponent();

	virtual void InitializeComponent() override;

	virtual void PerformMovement(float DeltaSeconds) override;

	void Duck();

	void UnDuck();

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void UnCrouch(bool bClientSimulation = false); //we need some custom functionality for the event where the character goes from crouch to duck

	bool IsDucking() const;

	bool bWantsToDuck : 1;

	bool bWantsToDuckFromCrouch : 1;

	/** Collision half-height when crouching (component scale is applied separately) */
	UPROPERTY(Category = "Character Movement (General Settings)", EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", UIMin = "0"))
		float DuckedHalfHeight;
	
};
