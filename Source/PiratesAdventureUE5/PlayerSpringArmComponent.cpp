// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSpringArmComponent.h"
#include "PiratesAdventureUE5.h"
#include "PlayerCharacter.h"
#include "PiratesAdventureFunctionLibrary.h"
#include "CineCameraComponent.h"

UPlayerSpringArmComponent::UPlayerSpringArmComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bRecordedCameraFrustum = false;
	bStartDelayFollowCam = false;
	FollowZOffset = 0;
	DefaultAperture = 0.1f;
	DefaultFocusDistance = 1700.f;

	bLockXAxis = false;
	bLockYAxis = false;
	bLockZAxis = false;

	bCamIsInitialized = false;
	bForceRerecordCameraFrustum = true;
	bWantsInitializeComponent = true;
}

void UPlayerSpringArmComponent::OnRegister()
{
	DefaultTargetArmLength = TargetArmLength;
	DesiredTargetArmLength = TargetArmLength;

	DefaultDesiredRotation = DesiredRotation;
	DesiredTargetRotation = DesiredRotation;

	DesiredAperture = DefaultAperture; 
	DesiredFocusDistance = DefaultFocusDistance;

	Super::OnRegister();
}

// Initializes the component, allowing us to register it with the player
void UPlayerSpringArmComponent::InitializeComponent()
{
	Super::InitializeComponent();

	BoomOwner = Cast<APlayerCharacter>(GetOwner());
	if (BoomOwner)
		UE_LOG(AnyLog, Warning, TEXT("Registered %s component with %s and saved the BoomOwner"), *(GetClass()->GetName()), *(BoomOwner->GetName()));
}

//We will implement our own location here
void UPlayerSpringArmComponent::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	//APiratesAdventureCharacter* BoomOwner = Cast<APiratesAdventureCharacter>(GetOwner());

	if (BoomOwner && !bDebug)
	{
		const float InverseCameraLagMaxTimeStep = (1.f / CameraLagMaxTimeStep);

		// Get the spring arm 'origin', the target we want to look at
		FVector ArmOrigin;
		if (FollowTarget != NULL)
		{
			ArmOrigin = FollowTarget->GetActorLocation() + TargetOffset;
		}
		else
		{
			ArmOrigin = GetComponentLocation() + TargetOffset;
		}

		FVector DesiredLoc = FVector::ZeroVector;
		float OriginalZ;

		if (bCamIsInitialized)
		{
			if (!bManualCameraTranslation)
			{
				//Interp the arm length if necessary
				if (TargetArmLength != DesiredTargetArmLength)
				{
					TargetArmLength = FMath::FInterpTo(TargetArmLength, DesiredTargetArmLength, DeltaTime, CameraArmLagSpeed);
					bRecordedCameraFrustum = false; //since target arm distance directly affects camera frustum, we need to update it if this is updating.
				}

				//Camera Jump Lock. if we are jumping, the camera should only follow when falling (after a certain offset) or when the character lands on a spot with a different Z value than before.
				if (BoomOwner->GetCharacterMovement()->MovementMode == MOVE_Falling)
				{
					//This is still kinda weird. Find a good number and keep in mind that negative numbers behave differently
					FVector OwnerDistance = FVector::ZeroVector;
					OwnerDistance.Z = BoomOwner->GetActorLocation().Z;
					FVector BottomFrustumDistance = FVector::ZeroVector;
					BottomFrustumDistance.Z = BottomFrustum.Z;

					if (FVector::Dist(OwnerDistance, BottomFrustumDistance) < FMath::Abs(FollowZOffset))
					{
						bStartDelayFollowCam = true;
					}

					if (bStartDelayFollowCam)
					{
						OriginalZ = BoomOwner->GetActorLocation().Z + FollowZOffset;
					}
					else
					{
						OriginalZ = PreviousDesiredLoc.Z;
					}

					bRecordedCameraFrustum = false;
				}
				else
				{
					//(Re)record camera frustum, so we can use it to determine falling distance.
					if (!bRecordedCameraFrustum
						|| bForceRerecordCameraFrustum)
					{
						FVector TempA, TempB;
						FVector ActorLoc = BoomOwner->GetActorLocation();
						ActorLoc.Z -= BoomOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
						UPiratesAdventureFunctionLibrary::GetPerspectiveCameraFrustumEdges(ActorLoc, BoomOwner->GetSideViewCameraComponent(), this, TopFrustum, TempA, BottomFrustum, TempB);
						FollowZOffset = FMath::Abs(ActorLoc.Z - BottomFrustum.Z) * FollowZRatio;
						bRecordedCameraFrustum = true;

						if (bForceRerecordCameraFrustum) bForceRerecordCameraFrustum = false;
					}

					bStartDelayFollowCam = false;
					OriginalZ = ArmOrigin.Z;
				}

				// We lag the target, not the actual camera position, so rotating the camera around does not have lag
				//For this game, we remove the conditional bLagCamera, and instead assume we always want to lag it. 
				DesiredLoc = ArmOrigin;

				//Update our camera lag speed to our owner's speed. If falling for too long, the character becomes too fast making it hard for the camera to catch up. 
				//Which is why we increase the lag speed so the camera catches up faster. 
				float BoomOwnerZVelocity = FMath::Abs(BoomOwner->GetCharacterMovement()->Velocity.Z);
				float FinalCameraLagSpeed = CameraLagSpeed;
				if (BoomOwnerZVelocity > 2048.f)
				{
					//First zero it out, clamp it and finally normalize it
					BoomOwnerZVelocity -= 2048.f;
					BoomOwnerZVelocity = FMath::Clamp(BoomOwnerZVelocity, 0.f, 2048.f);
					BoomOwnerZVelocity /= 2048.f;

					FinalCameraLagSpeed = CameraLagSpeed + ((MaxCameraLagSpeed - CameraLagSpeed) * BoomOwnerZVelocity);
				}

				if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && FinalCameraLagSpeed > 0.f)
				{
					const FVector ArmMovementStep = (ArmOrigin - PreviousArmOrigin) * (CameraLagMaxTimeStep / DeltaTime);
					FVector LerpTarget = PreviousArmOrigin;
					float RemainingTime = DeltaTime;
					while (RemainingTime > KINDA_SMALL_NUMBER)
					{
						const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
						LerpTarget += ArmMovementStep * (LerpAmount * InverseCameraLagMaxTimeStep);
						LerpTarget.Z = OriginalZ;
						RemainingTime -= LerpAmount;

						DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, FinalCameraLagSpeed);
						//Reset any axes that are locked down.
						DesiredLoc = GetLocationBasedOnLocks(PreviousDesiredLoc, DesiredLoc);
						PreviousDesiredLoc = DesiredLoc;
					}
				}
				else
				{
					DesiredLoc.Z = OriginalZ;
					DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, FinalCameraLagSpeed);
					//Reset any axes that are locked down
					DesiredLoc = GetLocationBasedOnLocks(PreviousDesiredLoc, DesiredLoc);
				}

				// Clamp distance if requested
				bool bClampedDist = false;
				if (CameraLagMaxDistance > 0.f)
				{
					const FVector FromOrigin = DesiredLoc - ArmOrigin;
					if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
					{
						DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
						//Reset any axes that are locked down
						DesiredLoc = GetLocationBasedOnLocks(PreviousDesiredLoc, DesiredLoc);
						bClampedDist = true;
					}
				}

				//Interp the rotation if necessary
				if (DesiredRotation != DesiredTargetRotation)
				{
					DesiredRotation = FMath::RInterpTo(DesiredRotation, DesiredTargetRotation, DeltaTime, CameraRotationLagSpeed);
				}
			}
			else //From here on is bManualCameraTranslation
			{
				if (!bManualCameraTranslationInstant)
				{
					DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, ManualCameraTranslationTarget, DeltaTime, ManualCameraTranslationLagSpeed);
					DesiredRotation = FMath::RInterpTo(DesiredRotation, ManualCameraRotation, DeltaTime, ManualCameraTranslationLagSpeed);
					if (ManualArmLengthTranslation != 0)
						TargetArmLength = FMath::FInterpTo(TargetArmLength, ManualArmLengthTranslation, DeltaTime, ManualCameraTranslationLagSpeed);
				}
				else
				{
					DesiredLoc = ManualCameraTranslationTarget;
					DesiredRotation = ManualCameraRotation;
					if (ManualArmLengthTranslation != 0)
						TargetArmLength = ManualArmLengthTranslation;
				}
			}

			//Calculate focus distance, for DOP reasons
			//Set focus distance outside of main calculation loop, since we do that regardless of manual translation or not.
			if (TrackingTarget != NULL)
			{
				FVector Distance = BoomOwner->GetSideViewCameraComponent()->GetComponentLocation() - TrackingTarget->GetActorLocation();
				BoomOwner->GetSideViewCameraComponent()->FocusSettings.ManualFocusDistance = FMath::FInterpTo(Distance.Size(), DesiredFocusDistance, DeltaTime, FocusLagSpeed);
			}
			else
			{
				BoomOwner->GetSideViewCameraComponent()->FocusSettings.ManualFocusDistance = FMath::FInterpTo(BoomOwner->GetSideViewCameraComponent()->FocusSettings.ManualFocusDistance, DesiredFocusDistance, DeltaTime, FocusLagSpeed);
			}

			//interp the cinecam aperture (controls the distance of the DOF)
			if (BoomOwner->GetSideViewCameraComponent()->CurrentAperture != DesiredAperture)
				BoomOwner->GetSideViewCameraComponent()->CurrentAperture = FMath::FInterpTo(BoomOwner->GetSideViewCameraComponent()->CurrentAperture, DesiredAperture, DeltaTime, FocusLagSpeed);
		}
		else
		{
			//first make sure we teleport the camera to the main character location. 
			DesiredLoc = ArmOrigin;
			DesiredRotation = DesiredTargetRotation;
		}

		PreviousDesiredRot = DesiredRotation;

		PreviousArmOrigin = ArmOrigin;
		PreviousDesiredLoc = DesiredLoc;

		DesiredLoc -= DesiredRotation.Vector() * TargetArmLength;
		DesiredLoc += FRotationMatrix(DesiredRotation).TransformVector(SocketOffset);

		FTransform WorldCamTM(DesiredRotation, DesiredLoc);
		FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentToWorld());

		// Update socket location/rotation
		RelativeSocketLocation = RelCamTM.GetLocation();
		RelativeSocketRotation = RelCamTM.GetRotation();

		UpdateChildTransforms();

		if (!bCamIsInitialized
			&& BoomOwner)
		{
			//Then, when we have updated our first transforms, update our camera frustum and disable camera initialization.
			FVector TempA, TempB;
			FVector ActorLoc = BoomOwner->GetActorLocation();
			ActorLoc.Z -= BoomOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			UPiratesAdventureFunctionLibrary::GetPerspectiveCameraFrustumEdges(ActorLoc, BoomOwner->GetSideViewCameraComponent(), this, TopFrustum, TempA, BottomFrustum, TempB);
			FollowZOffset = FMath::Abs(ActorLoc.Z - BottomFrustum.Z) * FollowZRatio;
			
			if(TopFrustum != FVector::ZeroVector
				&& BottomFrustum != FVector::ZeroVector)
			{
				//since we don't seem to get the frustums right away. May cause errors when our frustum happens to be equal to a zerovector.
				bRecordedCameraFrustum = true;
				bCamIsInitialized = true;
			}
		}
	}
	else
	{
		//Super it if we don't have a BoomOwner, so we can just get default behavior instead.
		//Super::UpdateDesiredArmLocation(bDoTrace, bDoLocationLag, bDoRotationLag, DeltaTime);
	}
}

FVector UPlayerSpringArmComponent::GetLocationBasedOnLocks(FVector OriginalLocation, FVector NewLocation)
{
	FVector ResultLocation = FVector::ZeroVector;

	ResultLocation.X = bLockXAxis ? OriginalLocation.X : NewLocation.X;
	ResultLocation.Y = bLockYAxis ? OriginalLocation.Y : NewLocation.Y;
	ResultLocation.Z = bLockZAxis ? OriginalLocation.Z : NewLocation.Z;

	return ResultLocation;
}

void UPlayerSpringArmComponent::SetCameraLock(bool bLocked)
{
	bLockXAxis = bLocked;
	bLockYAxis = bLocked;
	bLockZAxis = bLocked;
}

void UPlayerSpringArmComponent::StartManualCameraTranslation(FVector ManualLocation, FRotator ManualRotation, float ManualArmLength, bool bNoTransition, float TransitionSpeed, bool bShouldRotate, bool bIgnoreXAxis)
{
	ManualCameraTranslationTarget = ManualLocation;

	if (bIgnoreXAxis)
	{
		ManualCameraTranslationTarget.X = PreviousDesiredLoc.X;
	}

	ManualCameraTranslationLagSpeed = TransitionSpeed;
	ManualArmLengthTranslation = ManualArmLength;

	if (bShouldRotate)
	{
		ManualCameraRotation = ManualRotation;
	}
	else
	{
		ManualCameraRotation = DesiredRotation;
	}

	bManualCameraTranslationInstant = bNoTransition;
	bManualCameraTranslation = true;
}

void UPlayerSpringArmComponent::ResetCameraToDefault()
{
	DesiredTargetArmLength = DefaultTargetArmLength;
	DesiredTargetRotation = DefaultDesiredRotation;
	DesiredAperture = DefaultAperture;
	DesiredFocusDistance = DefaultFocusDistance;
}

void UPlayerSpringArmComponent::RerecordCameraFrustum()
{
	bForceRerecordCameraFrustum = true;
}