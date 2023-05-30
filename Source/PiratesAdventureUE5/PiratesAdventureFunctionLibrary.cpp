// Fill out your copyright notice in the Description page of Project Settings.


#include "PiratesAdventureFunctionLibrary.h"
#include "PiratesAdventureStaticLibrary.h"
#include "PiratesAdventureUE5.h"
#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "PiratesAdventureUE5LoadingScreen.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Modules/ModuleManager.h"

//--------------------------------------------------------------------------------------------------------
//  CAMERA MATH FUNCTIONS
//--------------------------------------------------------------------------------------------------------

void UPiratesAdventureFunctionLibrary::GetOrthographicCameraFrustumEdges(class UCineCameraComponent* CameraComponent, FVector& TopCenter, FVector& LeftCenter, FVector& BottomCenter, FVector& RightCenter)
{
	if (CameraComponent)
	{
		FMinimalViewInfo CameraView;
		CameraComponent->GetCameraView(0, CameraView);

		float width = CameraView.OrthoWidth;
		float height = width / CameraView.AspectRatio;

		TopCenter.Set(CameraView.Location.X, CameraView.Location.Y, CameraView.Location.Z + (height * 0.5f));
		BottomCenter.Set(CameraView.Location.X, CameraView.Location.Y, CameraView.Location.Z - (height * 0.5f));
		LeftCenter.Set(CameraView.Location.X, CameraView.Location.Y - (width * 0.5f), CameraView.Location.Z);
		RightCenter.Set(CameraView.Location.X, CameraView.Location.Y + (width * 0.5f), CameraView.Location.Z);
	}
}

void UPiratesAdventureFunctionLibrary::GetPerspectiveCameraFrustumEdges(FVector OwnerWorldLocation, class UCineCameraComponent* CameraComponent, class USpringArmComponent* SpringArmComponent, FVector& TopCenter, FVector& LeftCenter, FVector& BottomCenter, FVector& RightCenter)
{
	if (CameraComponent
		&& SpringArmComponent
		&& SpringArmComponent->TargetArmLength > 0
		&& CameraComponent->GetWorld())
	{
		FMinimalViewInfo CameraView;
		CameraComponent->GetCameraView(0, CameraView);
		float CameraDistance = SpringArmComponent->TargetArmLength;
		FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(CameraComponent->GetWorld());

		//Just in case to prevent division by zero. Besides, if the viewportsize is zero, we weren't able to get the data anyway. 
		if (ViewportSize.X == 0 || ViewportSize.Y == 0)
			return;

		float hFOV = FMath::DegreesToRadians(CameraComponent->GetHorizontalFieldOfView());
		float vFOV = 2 * FMath::Atan(FMath::Tan(hFOV * 0.5) * (ViewportSize.Y / ViewportSize.X));
		float vFOVoffset_top = 0; //in case our camera has an angle, we need to calculate an offset.
		float vFOVoffset_bottom = 0; //in case our camera has an angle, we need to calculate an offset.
		float halfheight = FMath::Tan(vFOV / 2) * CameraDistance;
		float halfwidth = FMath::Tan(hFOV / 2) * CameraDistance;

		//calculate the offset if our springarm has a pitch rotation.
		//We need two different lengths; one using CameraDistance as hypotenuse (long) and one using the halfheight as hypotenuse (short). 
		//Depending on the camera angle, we need to offset both the bottom and top Z with either the short or the long offset.
		if (SpringArmComponent->PreviousDesiredRot.Pitch != 0)
		{
			//if our camera views from upwards, we need the bottom offset to be long and the top to be short.
			if (SpringArmComponent->PreviousDesiredRot.Pitch < 0)
			{
				float CameraAngle = FMath::Abs(SpringArmComponent->PreviousDesiredRot.Pitch);
				vFOVoffset_top = ((CameraDistance * FMath::Cos(vFOV * 0.25)) * FMath::Acos(vFOV * 0.25)) * 0.5; //top one is short one
				vFOVoffset_bottom = (CameraDistance / FMath::Sin(90.f - CameraAngle - (vFOV * 0.5))) * FMath::Sin(vFOV * 0.5); //bottom one is long one
			}
			else
			{
				float CameraAngle = FMath::Abs(SpringArmComponent->PreviousDesiredRot.Pitch);
				vFOVoffset_bottom = ((CameraDistance * FMath::Cos(vFOV * 0.25)) * FMath::Acos(vFOV * 0.25)) * 0.5; //top one is short one
				vFOVoffset_top = (CameraDistance / FMath::Sin(90.f - CameraAngle - (vFOV * 0.5))) * FMath::Sin(vFOV * 0.5); //bottom one is long one
			}
		}
		else
		{
			//if there's no pitch rotation, we can just use the halfheight.
			vFOVoffset_top = halfheight;
			vFOVoffset_bottom = halfheight;
		}

		TopCenter.Set(OwnerWorldLocation.X, OwnerWorldLocation.Y, OwnerWorldLocation.Z + vFOVoffset_top);
		BottomCenter.Set(OwnerWorldLocation.X, OwnerWorldLocation.Y, OwnerWorldLocation.Z + vFOVoffset_bottom); //since the result is negative, we add it to keep it negative.
		LeftCenter.Set(OwnerWorldLocation.X, OwnerWorldLocation.Y + halfwidth, OwnerWorldLocation.Z);
		RightCenter.Set(OwnerWorldLocation.X, OwnerWorldLocation.Y - halfwidth, OwnerWorldLocation.Z);
	}
}

//-------------------------------------------------------------------------------------------------------
// DEADZONE MATH FUNCTIONS
//-------------------------------------------------------------------------------------------------------

float UPiratesAdventureFunctionLibrary::GetModifiedFloat(float Input, float DeadzoneLow, float DeadzoneHigh)
{
	float Output = 0.f;

	Output = FMath::Clamp(FMath::Abs(Input) - DeadzoneLow, 0.f, DeadzoneHigh);

	if (Output <= 0)
		return 0.f;

	return FMath::Clamp(Output / (DeadzoneHigh = DeadzoneLow), 0.f, 1.f);
}

float UPiratesAdventureFunctionLibrary::GetInputStrength(float InputX, float InputY, float DeadzoneLow, float DeadzoneHigh)
{
	FVector2D Input = FVector2D(InputX, InputY);
	float ModFloat = GetModifiedFloat(Input.Size(), DeadzoneLow, DeadzoneHigh);
	Input.Normalize();

	if (Input.Size() == 0.f)
		return 0.f;

	return FMath::Clamp(ModFloat / Input.Size(), 0.f, 1.f);
}

void UPiratesAdventureFunctionLibrary::GetCorrectedInput(float InputX, float InputY, float DeadzoneLow, float DeadzoneHigh, float& OutputX, float& OutputY)
{
	FVector2D Input = FVector2D(InputX, InputY);
	Input.Normalize();
	Input *= GetInputStrength(InputX, InputY, DeadzoneLow, DeadzoneHigh);

	OutputX = Input.X;
	OutputY = Input.Y;
}

//-------------------------------------------------------------------------------------------------------
// LOADING SCREEN FUNCTIONS: MOVE ALL TO LOADING SCREEN MODULE
//-------------------------------------------------------------------------------------------------------

void UPiratesAdventureFunctionLibrary::PlayLoadingScreen(UGameInstance* WidgetOwner, TSubclassOf<UUserWidget> Widget, bool bPlayUntilStopped, float PlayTime)
{
	//UUserWidget* NewWidget = CreateWidget<UUserWidget>(WidgetOwner, Widget, TEXT("Loading Screen"));
	//FPiratesAdventureUE5LoadingScreenModule& LoadingScreenModule = FPiratesAdventureUE5LoadingScreenModule::Get();
	//LoadingScreenModule.StartInGameLoadingScreen(nullptr, bPlayUntilStopped, PlayTime);
}

void UPiratesAdventureFunctionLibrary::StopLoadingScreen()
{
	//FPiratesAdventureUE5LoadingScreenModule& LoadingScreenModule = FPiratesAdventureUE5LoadingScreenModule::Get();
	//LoadingScreenModule.StopInGameLoadingScreen();
}

//bool UPiratesAdventureFunctionLibrary::IsInEditor()
//{
//	return GIsEditor;
//}

bool UPiratesAdventureFunctionLibrary::GetIsModuleLoaded(FName ModuleName)
{
	return FModuleManager::Get().IsModuleLoaded(ModuleName);
}

//--------------------------------------------------------------------------------------------------------
//  MODIFIED DAMAGE FUNCTIONS
//--------------------------------------------------------------------------------------------------------

bool UPiratesAdventureFunctionLibrary::ApplyRadialDamageWithFalloffAllActors(const UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser, AController* InstigatedByController, EDamagePenetration PenetrationType, ECollisionChannel DamagePreventionChannel)
{
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage), false, DamageCauser);

	SphereParams.AddIgnoredActors(IgnoreActors);

	// query scene to see what we hit
	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllObjects), FCollisionShape::MakeSphere(DamageOuterRadius), SphereParams);
	}

	// collate into per-actor list of hit components
	TMap<AActor*, TArray<FHitResult> > OverlapComponentMap;
	for (int32 Idx = 0; Idx < Overlaps.Num(); ++Idx)
	{
		FOverlapResult const& Overlap = Overlaps[Idx];
		AActor* const OverlapActor = Overlap.GetActor();

		if (OverlapActor &&
			OverlapActor->bCanBeDamaged &&
			OverlapActor != DamageCauser &&
			Overlap.Component.IsValid())
		{
			FHitResult Hit;

			FVector CompLoc = Overlap.Component.Get()->GetComponentLocation();
			FVector ObjectDistance = Origin - CompLoc;

			//First process our penetration type. If the damage penetrates, make a cheap hitresult with approximate impact data to save on raycasting
			if (PenetrationType == EDamagePenetration::DP_PenetrateAll
				|| (PenetrationType == EDamagePenetration::DP_PenetrateCoreOnly && ObjectDistance.Size() < DamageInnerRadius)
				|| (PenetrationType == EDamagePenetration::DP_PenetrateOuterOnly && ObjectDistance.Size() > DamageInnerRadius && ObjectDistance.Size() < DamageOuterRadius))
			{
				FVector const FakeHitNorm = (Origin - CompLoc).GetSafeNormal();		// normal points back toward the epicenter
				Hit = FHitResult(OverlapActor, Overlap.Component.Get(), CompLoc, FakeHitNorm);

				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
			else if (DamagePreventionChannel == ECC_MAX || ComponentIsDamageableFrom(Overlap.Component.Get(), Origin, DamageCauser, IgnoreActors, DamagePreventionChannel, Hit))
			{
				TArray<FHitResult>& HitList = OverlapComponentMap.FindOrAdd(OverlapActor);
				HitList.Add(Hit);
			}
		}
	}

	bool bAppliedDamage = false;

	UE_LOG(AnyLog, Warning, TEXT("OVERLAPPING COMPONENTS IN OUR MAP IS %d"), OverlapComponentMap.Num());

	if (OverlapComponentMap.Num() > 0)
	{
		// make sure we have a good damage type
		TSubclassOf<UDamageType> const ValidDamageTypeClass = DamageTypeClass ? DamageTypeClass : TSubclassOf<UDamageType>(UDamageType::StaticClass());

		FRadialDamageEvent DmgEvent;
		DmgEvent.DamageTypeClass = ValidDamageTypeClass;
		DmgEvent.Origin = Origin;
		DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, DamageFalloff);

		// call damage function on each affected actors
		for (TMap<AActor*, TArray<FHitResult> >::TIterator It(OverlapComponentMap); It; ++It)
		{
			AActor* const Victim = It.Key();
			TArray<FHitResult> const& ComponentHits = It.Value();
			DmgEvent.ComponentHits = ComponentHits;

			Victim->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);

			bAppliedDamage = true;
		}
	}

	return bAppliedDamage;
}

/** @RETURN True if weapon trace from Origin hits component VictimComp.  OutHitResult will contain properties of the hit. */
bool UPiratesAdventureFunctionLibrary::ComponentIsDamageableFrom(UPrimitiveComponent* VictimComp, FVector const& Origin, AActor const* IgnoredActor, const TArray<AActor*>& IgnoreActors, ECollisionChannel TraceChannel, FHitResult& OutHitResult)
{
	FCollisionQueryParams LineParams(SCENE_QUERY_STAT(ComponentIsVisibleFrom), true, IgnoredActor);
	LineParams.AddIgnoredActors(IgnoreActors);

	// Do a trace from origin to middle of box
	UWorld* const World = VictimComp->GetWorld();
	check(World);

	FVector const TraceEnd = VictimComp->Bounds.Origin;
	FVector TraceStart = Origin;
	if (Origin == TraceEnd)
	{
		// tiny nudge so LineTraceSingle doesn't early out with no hits
		TraceStart.Z += 0.01f;
	}
	bool const bHadBlockingHit = World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, LineParams);
	//::DrawDebugLine(World, TraceStart, TraceEnd, FLinearColor::Red, true);

	// If there was a blocking hit, it will be the last one
	if (bHadBlockingHit)
	{
		if (OutHitResult.Component == VictimComp)
		{
			// if blocking hit was the victim component, it is visible
			return true;
		}
		else
		{
			// if we hit something else blocking, it's not
			UE_LOG(AnyLog, Warning, TEXT("Radial Damage to %s blocked by %s (%s)"), *GetNameSafe(VictimComp), *GetNameSafe(OutHitResult.GetActor()), *GetNameSafe(OutHitResult.Component.Get()));
			return false;
		}
	}

	// didn't hit anything, assume nothing blocking the damage and victim is consequently visible
	// but since we don't have a hit result to pass back, construct a simple one, modeling the damage as having hit a point at the component's center.
	FVector const FakeHitLoc = VictimComp->GetComponentLocation();
	FVector const FakeHitNorm = (Origin - FakeHitLoc).GetSafeNormal();		// normal points back toward the epicenter
	OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp, FakeHitLoc, FakeHitNorm);
	return true;
}