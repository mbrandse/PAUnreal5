// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMovementComponent.h"
#include "PiratesAdventureUE5.h"
#include "PlayerCharacter.h"
#include "Components/CapsuleComponent.h"

//Cycle stat groups
DECLARE_CYCLE_STAT(TEXT("Char PerformMovement"), STAT_CharacterMovementPerformMovement, STATGROUP_Character);

// Statics, taken from CharacterMovementComponent
namespace CharacterMovementComponentStatics
{
	static const FName CrouchTraceName = FName(TEXT("CrouchTrace"));
	static const FName FindWaterLineName = FName(TEXT("FindWaterLine"));
	static const FName FallingTraceParamsTag = FName(TEXT("PhysFalling"));
	static const FName CheckLedgeDirectionName = FName(TEXT("CheckLedgeDirection"));
	static const FName ProjectLocationName = FName(TEXT("NavProjectLocation"));
	static const FName CheckWaterJumpName = FName(TEXT("CheckWaterJump"));
	static const FName ComputeFloorDistName = FName(TEXT("ComputeFloorDistSweep"));
	static const FName FloorLineTraceName = FName(TEXT("ComputeFloorDistLineTrace"));
	static const FName ImmersionDepthName = FName(TEXT("MovementComp_Character_ImmersionDepth"));
}

UPlayerMovementComponent::UPlayerMovementComponent()
{

}

void UPlayerMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

bool UPlayerMovementComponent::IsDucking() const
{
	APlayerCharacter* Character = Cast<APlayerCharacter>(CharacterOwner);

	return Character && Character->bIsDucked;
}

void UPlayerMovementComponent::PerformMovement(float DeltaSeconds)
{
	Super::PerformMovement(DeltaSeconds);

	SCOPE_CYCLE_COUNTER(STAT_CharacterMovementPerformMovement);

	FVector OldVelocity;
	FVector OldLocation;

	{
		FScopedMovementUpdate ScopedMovementUpdate(UpdatedComponent, bEnableScopedMovementUpdates ? EScopedUpdate::DeferredUpdates : EScopedUpdate::ImmediateUpdates);

		MaybeUpdateBasedMovement(DeltaSeconds);

		OldVelocity = Velocity;
		OldLocation = UpdatedComponent->GetComponentLocation();

		ApplyAccumulatedForces(DeltaSeconds);

		const bool bAllowedToCrouch = CanCrouchInCurrentState();
		if (!bWantsToDuck && IsDucking())
		{
			UnDuck();
		}
		else if (bWantsToDuck && bAllowedToCrouch && !IsDucking())
		{
			Duck();
		}
	}
}

void UPlayerMovementComponent::Duck()
{
	if (!HasValidData())
	{
		return;
	}

	if (!CanCrouchInCurrentState())
	{
		return;
	}

	APlayerCharacter* Character = Cast<APlayerCharacter>(CharacterOwner);

	if (!Character)
	{
		return;
	}

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DuckedHalfHeight)
	{
		Character->bIsDucked = true;
		Character->OnStartCrouch(0.f, 0.f);
		return;
	}

	// Change collision size to crouching dimensions
	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	// Height is not allowed to be smaller than radius.
	const float ClampedDuckedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, DuckedHalfHeight);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedDuckedHalfHeight);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedDuckedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// Crouching to a larger height? (this is rare)
	if (ClampedDuckedHalfHeight > OldUnscaledHalfHeight)
	{
		FCollisionQueryParams CapsuleParams(CharacterMovementComponentStatics::CrouchTraceName, false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);
		const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
			UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

		// If encroached, cancel
		if (bEncroached)
		{
			CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
			return;
		}
	}

	if (bCrouchMaintainsBaseLocation)
	{
		// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
		UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true);
	}

	Character->bIsDucked = true;

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedDuckedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	UE_LOG(AnyLog, Warning, TEXT("TRYING TO DUCK -> bWantsToDuck %d bWantsToCrouch %d"), bWantsToDuck, bWantsToCrouch);
	CharacterOwner->OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	Character->OnPlayerDuck();
}

void UPlayerMovementComponent::UnDuck()
{
	if (!HasValidData())
	{
		return;
	}

	APlayerCharacter* Character = Cast<APlayerCharacter>(CharacterOwner);

	if (!Character)
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		Character->bIsDucked = false;
		Character->OnEndCrouch(0.f, 0.f);
		Character->OnPlayerUnduck();
		return;
	}

	const float CurrentDuckedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to unducked size.
	check(CharacterOwner->GetCapsuleComponent());
	bool bUpdateOverlaps = false;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);

	// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(CharacterMovementComponentStatics::CrouchTraceName, false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation); // Shrink by negative amount, so actually grow it.
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	if (!bCrouchMaintainsBaseLocation)
	{
		// Expand in place
		bEncroached = GetWorld()->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			// Try adjusting capsule position to see if we can avoid encroachment.
			if (ScaledHalfHeightAdjust > 0.f)
			{
				// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
				float PawnRadius, PawnHalfHeight;
				CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
				const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
				const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
				const FVector Down = FVector(0.f, 0.f, -TraceDist);

				FHitResult Hit(1.f);
				const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
				const bool bBlockingHit = GetWorld()->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
				if (Hit.bStartPenetrating)
				{
					bEncroached = true;
				}
				else
				{
					// Compute where the base of the sweep ended up, and see if we can stand there
					const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
					const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + PawnHalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
					bEncroached = GetWorld()->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					if (!bEncroached)
					{
						// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
						UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false);
					}
				}
			}
		}
	}
	else
	{
		// Expand while keeping base location the same.
		FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentDuckedHalfHeight);
		bEncroached = GetWorld()->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			if (IsMovingOnGround())
			{
				// Something might be just barely overhead, try moving down closer to the floor to avoid it.
				const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
				if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
				{
					StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
					bEncroached = GetWorld()->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
				}
			}
		}

		if (!bEncroached)
		{
			// Commit the change in location.
			UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false);
			bForceNextFloorCheck = true;
		}
	}

	// If still encroached then abort.
	if (bEncroached)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), OldUnscaledHalfHeight, false);
		return;
	}

	Character->bIsDucked = false;

	// now call SetCapsuleSize() to cause touch/untouch events
	bUpdateOverlaps = true;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	UE_LOG(AnyLog, Warning, TEXT("TRYING TO UNDUCK -> bWantsToDuck %d bWantsToCrouch %d"), bWantsToDuck, bWantsToCrouch);
	CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	Character->OnPlayerUnduck();
	//TODO: add an OnEndDuck in the character?
}

void UPlayerMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);

	APlayerCharacter* Character = Cast<APlayerCharacter>(CharacterOwner);

	if (Character)
	{
		Character->bIsDucked = true;
	}

	UE_LOG(AnyLog, Warning, TEXT("TRYING TO CROUCH -> bWantsToDuck %d bWantsToCrouch %d"), bWantsToDuck, bWantsToCrouch);
	Character->OnPlayerCrouch();
}

void UPlayerMovementComponent::UnCrouch(bool bClientSimulation)
{
	static int Tester;

	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	APlayerCharacter* Character = Cast<APlayerCharacter>(CharacterOwner);

	if (!Character)
	{
		return;
	}

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			CharacterOwner->bIsCrouched = false;
		}
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		Character->OnPlayerUncrouch();
		return;
	}

	const float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	//for ducking variables
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float ClampedDuckedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, DuckedHalfHeight);

	//Grow to uncrouched size, or ducked size when the player is still ducking.
	check(CharacterOwner->GetCapsuleComponent());
	bool bUpdateOverlaps = false;

	//this is to check whether there is encrouching going on. 
	if (IsDucking())
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedDuckedHalfHeight, bUpdateOverlaps);
	}
	else
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);
	}

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(CharacterMovementComponentStatics::CrouchTraceName, false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bCrouchMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = GetWorld()->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = GetWorld()->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + PawnHalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = GetWorld()->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
			bEncroached = GetWorld()->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = GetWorld()->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), OldUnscaledHalfHeight, false);
			return;
		}

		CharacterOwner->bIsCrouched = false;
		Character->bIsDucked = false;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	// now call SetCapsuleSize() to cause touch/untouch events
	bUpdateOverlaps = true;

	if (IsDucking())
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedDuckedHalfHeight, bUpdateOverlaps);
	}
	else
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), bUpdateOverlaps);
	}

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();

	if (IsDucking())
	{
		//Since the effects on OnEndCrouch actually don't have any functionality until the blueprint implementable event, let's set the relative location here directly. 
		FVector RelativeLoc = CharacterOwner->GetMesh()->GetRelativeLocation();
		RelativeLoc.Z -= ClampedDuckedHalfHeight - GetCrouchedHalfHeight();
		CharacterOwner->GetMesh()->SetRelativeLocation(RelativeLoc);
	}
	else
	{
		UE_LOG(AnyLog, Warning, TEXT("TRYING TO UNCROUCH -> bWantsToDuck %d bWantsToCrouch %d"), bWantsToDuck, bWantsToCrouch);
		CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
		Character->OnPlayerUncrouch();
	}

	// Don't smooth this change in mesh position
	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData && ClientData->MeshTranslationOffset.Z != 0.f)
		{
			ClientData->MeshTranslationOffset += FVector(0.f, 0.f, MeshAdjust);
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

