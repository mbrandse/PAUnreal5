// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveObjectBase.h"
#include "InterfaceContextAction.h"
#include "InteractiveObjectContext.generated.h"

UCLASS()
class PIRATESADVENTUREUE5_API AInteractiveObjectContext : public AInteractiveObjectBase, public IInterfaceContextAction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractiveObjectContext();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
