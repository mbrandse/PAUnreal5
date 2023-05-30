// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
#pragma once

//Core
#include "RamaMeleeCore.h"
 
#include "Components/SkeletalMeshComponent.h"
#include "RamaMeleeWeapon.generated.h"
 
/** You can create an array of these in early testing phases when you just want have 1 character and many weapons */
USTRUCT(BlueprintType)
struct FRamaMeleeWeaponInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	USkeletalMesh* Mesh;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	FName AttachSocket = NAME_None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	FTransform RelativeTransform;
	 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	FRamaMeleeDamageMap DamageMap;
}; 

/** Final Game Production: Use this if you have many characters wanting to use the same weapon with same damage mappings! ♥ Rama*/
UCLASS(BlueprintType)
class URamaMeleeWeaponData : public UDataAsset
{ 
	GENERATED_BODY()
public:	
	 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	FRamaMeleeWeaponInfo Data;
};

/** Use this to give each character BP access to the whole list of melee weapons + damage maps with a single click! */
UCLASS(BlueprintType)
class URamaMeleeAllWeaponData : public UDataAsset
{ 
	GENERATED_BODY()
public:	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Rama Melee Plugin")
	TArray<URamaMeleeWeaponData*> AllDatas;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams( FRamaMeleeHitSignature, class AActor*, HitActor, class UPrimitiveComponent*, HitComponent, const FVector&, ImpactPoint, const FVector&, ImpactNormal, int32, ShapeIndex, FName, HitBoneName, const struct FHitResult&, HitResult );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FRamaMeleeChaosEngineFixSignature, class URamaMeleeWeapon*, RamaMeleeComp); 

UCLASS(ClassGroup=Rama, BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class RAMAMELEEPLUGIN_API URamaMeleeWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	URamaMeleeWeapon(const FObjectInitializer& ObjectInitializer); 
	
	UPROPERTY(EditAnywhere, Category="Rama Melee Weapon")
	TArray<TEnumAsByte<EObjectTypeQuery> > MeleeTraceObjectTypes;
	 
	/**If HitWasDamage returns false, then the weapon hit another object with a non-damaging shape. Returns false if no hit at all. */
	UPROPERTY(BlueprintAssignable, Category="Rama Melee Weapon")
	FRamaMeleeHitSignature RamaMeleeWeapon_OnHit;
		
	/** 
		Should initial overlaps count as hits? If SwingStart happens when the weapon mesh is already overlapping another actor, and this is true, then a hit event will occur! 
		
		This can be changed during runtime based on your desired behavior for the weapon based on its current state during gametime! 
		
		♥ Rama
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool HitEvents_IncludeInitialOverlapsDuringSwingStart = false;
	
	
	/** Draw the PhysX Shapes, indicating which ones do damage! <3 Rama */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawShapes = true;
	
	/** 
		To determine which indices should go in the DamageMap, please turn this on, 
		
		And then you will see the display of the indices during PlayTime In Editor! (PTIE)! 
			(yes there's a certain kind of joke in here somewhere)
		
		A display of 0:0 means the first body of the skeletal mesh and the first shape of that body
		
		A display of 2:5 means the third body of the skeletal mesh and the 6th shape
		
		The numbers match the way the Damage Map should look.
		
		So if you see 3:2, you should create a Damage map that uses the same numbers, body at index 3 (4th body) and shape at index 2 (3rd shape)

		♥ Rama
		
		PS: the BP viewport version may or may not match due to changes in the physics code in UE5, please go in game via PTIE to know for sure!
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawShapeIndices = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawSweeps = true;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	bool DrawLines = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	float DrawShapes_Thickness = 3;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon ~ Draw")
	float DrawShapes_Duration = 2;
	
	/**
		If you enable this feature, more traces will be performed, so that within a single trace, even if a hit occurs, I will perform the trace again in that same tick, ignoring the hit actor, and finding any other actors that are within the same trace path.
		
		This resolves an issue for fast swings where multiple objects get hit in the same trace, but only 1 hit is reported, when you have a weapon that is supposed to pass through each hit object rather than stopping on the first hit.
		
		The physX engine stops on the first hit, so I must create another trace to keep finding other objects along the same single trace path.
		
		Potential disadvantage of this feature is that more traces will be performed, but it will only happen for the number of hits that occur, the number of unique objects along the trace path.
		
		Please note this feature does _not_ result in multiple hit reports on the same object, within the same trace, because I ignore each hit object and continue trace looking for other unique objects.
		
		If your melee weapon design expects to recoil or stop on the first valid hit, you will not need this feature.
		
		This is a per-weapon feature in case you have some weapons that DO cut through all possible objects, and other weapons that stop on the first hit object.
		
		Enjoy!
		
		♥
		
		Rama
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	bool PerformDeepTrace = false;
	
	/** If this is true, even if a weapon has multiple damaging bodies, only the first damaging body will be swept against all possible actors along the swing path, for performance reasons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	bool DeepTrace_SingleBodyMode = false;
	
	/** 
		With the advent of UE5 (♥♥♥ Yaaaaaay!!!! ♥♥♥),
		
		The DamageMap had to be changed to cooperate with the UE5 Physics Chaos system.
		
		I make the process easier now actually, because I display the index of each shape in the viewport of the Blueprint!
		
		Turn off DrawShapeIndices if you do not need to see the indices any more!
		
		Whichever shapes you want to do damage, should be added to this list!
		Enjoy!
		
		♥
		
		Rama
	
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	FRamaMeleeDamageMap DamageMap;
	
//~~~~~~~~~~~~~
// Core Functions
//~~~~~~~~~~~~~
public:
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void StartSwingDamage();
	
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void StopSwingDamage();
	
	/** Reset and rebuild Swing Pose List from current number of bodies in skeletal mesh asset */
	UFUNCTION(Category = "Rama Melee Weapon", BlueprintCallable)
    void UpdateSwingPoseBodies();
	
	/** 
		Not meant for final production, meant for early testing before you create all your data assets.
	
		Please re-attach this RamaMeleeComponent yourself using the AttachSocket via your preffered method (snap or RelativeTransform)
	*/
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void SetRamaMeleeWeaponFromInfo(const FRamaMeleeWeaponInfo& Info)
	{
		if(Info.Mesh != nullptr)
		{
			SetSkeletalMesh(Info.Mesh);
			DamageMap = Info.DamageMap;
		}
	}
	
	/** Final Game Production: Use this if you have many characters wanting to use the same weapons with same damage mappings! 
	
		To create data assets:
		1. Right click to create a new asset
		2. Misc
		3. Data Asset
		4. URamaMeleeWeaponData or URamaMeleeAllWeaponData once you have a bunch of individual melee weapons configured!
		
		5. Please re-attach this RamaMeleeComponent yourself using the AttachSocket via your preffered method (snap or RelativeTransform)
		
		♥ Rama
	*/
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	void SetRamaMeleeWeapon(URamaMeleeWeaponData* DataAsset)
	{
		if(DataAsset)
		{
			SetRamaMeleeWeaponFromInfo(DataAsset->Data);
		}
	}
	
//Chaos Engine Issue
public:
	
	/** 
		How frequently should I perform my Chaos Engine repair on the skeletal mesh component whose shapes keep disappearing for unknown reasons?
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rama Melee Weapon")
	float RamaChaosEngineRepairMaxFrequencySeconds = 0.1;
	
	/** 
		This happens at random times, Epic should investigate, my own code is read-only, something else randomly causes the skeletal mesh to have the shape data go invalid 
	
		The solution is simply to recreate the physics state of the current weapon, then the invalidated shapes are restored to full health.
		
		I suspect that the BodyInstance::GetAllShapes_AssumesLocked function is actually changing the data it is returning somehow.
		
		If not that, then for some other reason the physics shapes are randomly disappearing, and especially during weld/attach or skeletal mesh change operations.
		
		See Core_PhysX.cpp for more information on how I am detecting exactly when the shapes are going invalid!
		
		This was quite a research project! 
		
		Because the Chaos engine uses GetObjectChecked, which insta-crashes, this does not give me any time to see if the shape data is invalid myself and respond properly.
		
		Using my own custom code I can gracefully exit and attempt a fix, which currently is RecreatePhysicsState()
		
		This event is just so you as the Beautiful UE5 developer know what I am doing under the hood, you can create a particle effect or print string off of this event,
		Just so you are aware every time I am compelled to recreate the physics state.
		
		Use RamaChaosEngineRepairMaxFrequencySeconds to control how frequently this event can occur!
		 
		♥
		
		Rama
	*/
	UPROPERTY(BlueprintAssignable, Category="Rama Melee Weapon")
	FRamaMeleeChaosEngineFixSignature RamaMeleeWeapon_ChaosEngineFix;
	 
	void RamaReportsChaosPhysicsShapesWentInvalidOnUs();
	
	void RamaChaosEngineRepair();
	
	float RamaChaosEngineLastRepairTime = 0;
	
//~~~~~~~~~~~~~
// Core Utility
//~~~~~~~~~~~~~
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Rama Melee Weapon")
	bool DoingSwingTraces = false;
	 
	/** Returns false if no hit at all. */
	UFUNCTION(Category="Rama Melee Weapon",BlueprintCallable)
	bool MeleeSweep(FHitResult& Hit, const TArray<FTransform>& BodyPreviousPose);
	
	TArray<FTransform> SwingPrevPose;
	void SwingTick();
	
public:
	void Draw();
	 
	FORCEINLINE bool IsValid() const
	{
		return SkeletalMesh != nullptr;
	}
	
	virtual void InitializeComponent() override;
	
	virtual void SetSkeletalMesh(class USkeletalMesh* NewMesh, bool bReinitPose = true) override;
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//Tick
	//		By using tick to draw, it shows up in Editor Preview! Woohoo!
	//			-Rama
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override
	{
		Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		
		if(DoingSwingTraces)
		{
			SwingTick();
		}
		 
		//Draw
		if(DrawShapes)
		{  
			Draw();
		}
	}
};



