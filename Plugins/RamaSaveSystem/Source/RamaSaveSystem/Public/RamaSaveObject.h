// Copyright 2018 by Nathan "Rama" Iyer. All Rights Reserved.

#pragma once
  
#include "RamaSaveObject.generated.h"

UCLASS(abstract, Blueprintable, BlueprintType)
class RAMASAVESYSTEM_API URamaSaveObject : public UObject
{
	GENERATED_BODY()
	
public:	
	/** 
		This is the Game Version that was loaded from disk for the current save file.
		
		Upon saving to the same file, the old version is replaced with current, as determined in Project Settings -> Rama Save System -> Game Version
		
		-Rama
	*/
	UPROPERTY(Category = "Rama Save System", EditInstanceOnly, BlueprintReadOnly)
	float GameVersion = 1.0;
	

};