// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiratesAdventureFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PIRATESADVENTUREUE5_API UPiratesAdventureFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//--------------------------------------------------------------------------------------------------------
	//  CAMERA MATH FUNCTIONS
	//--------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Camera")
		static void GetOrthographicCameraFrustumEdges(class UCineCameraComponent* CameraComponent, FVector& TopCenter, FVector& LeftCenter, FVector& BottomCenter, FVector& RightCenter);

	UFUNCTION(BlueprintCallable, Category = "Camera")
		static void GetPerspectiveCameraFrustumEdges(FVector OwnerWorldLocation, class UCineCameraComponent* CameraComponent, class USpringArmComponent* SpringArmComponent, FVector& TopCenter, FVector& LeftCenter, FVector& BottomCenter, FVector& RightCenter);
	
	//--------------------------------------------------------------------------------------------------------
	//  DEADZONE MATH FUNCTIONS
	//--------------------------------------------------------------------------------------------------------
	
	UFUNCTION(BlueprintPure, Category = "Deadzone Correction")
		static float GetModifiedFloat(float Input, float DeadzoneLow, float DeadzoneHigh);

	UFUNCTION(BlueprintPure, Category = "Deadzone Correction")
		static float GetInputStrength(float InputX, float InputY, float DeadzoneLow, float DeadzoneHigh);

	UFUNCTION(BlueprintPure, Category = "Deadzone Correction")
		static void GetCorrectedInput(float InputX, float InputY, float DeadzoneLow, float DeadzoneHigh, float& OutputX, float& OutputY);

	//--------------------------------------------------------------------------------------------------------
	//  LOADING SCREEN FUNCTIONS: MOVE ALL TO LOADING SCREEN MODULE
	//--------------------------------------------------------------------------------------------------------

	/** Show the native loading screen, such as on a map transfer. If bPlayUntilStopped is false, it will be displayed for PlayTime and automatically stop */
	UFUNCTION(BlueprintCallable, Category = "Loading")
		static void PlayLoadingScreen(class UGameInstance* WidgetOwner, TSubclassOf<class UUserWidget> Widget, bool bPlayUntilStopped, float PlayTime);

	/** Turns off the native loading screen if it is visible. This must be called if bPlayUntilStopped was true */
	UFUNCTION(BlueprintCallable, Category = "Loading")
		static void StopLoadingScreen();

	///** Returns true if this is being run from an editor preview */
	//UFUNCTION(BlueprintPure, Category = "Loading")
	//	static bool IsInEditor();

	/** Returns true if this is being run from an editor preview */
	UFUNCTION(BlueprintPure, Category = "Loading")
		static bool GetIsModuleLoaded(FName ModuleName);

	//--------------------------------------------------------------------------------------------------------
	//  MODIFIED DAMAGE FUNCTIONS
	//--------------------------------------------------------------------------------------------------------

	//Modified Radial Damage with Falloff that checks for all actors, rather than just dynamic ones. Is heavier, but should be usable on non-Physics objects too. 
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Damage", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreActors"))
		static bool ApplyRadialDamageWithFalloffAllActors(const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = NULL, AController* InstigatedByController = NULL, EDamagePenetration PenetrationType = EDamagePenetration::DP_PenetrateNone, ECollisionChannel DamagePreventionChannel = ECC_Visibility);

	static bool ComponentIsDamageableFrom(UPrimitiveComponent* VictimComp, FVector const& Origin, AActor const* IgnoredActor, const TArray<AActor*>& IgnoreActors, ECollisionChannel TraceChannel, FHitResult& OutHitResult);
};
