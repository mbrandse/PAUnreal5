// Copyright 2022 by Nathan "Rama" Iyer. All Rights Reserved.
//#include "RamaMeleePluginPrivatePCH.h"
 
#include "RamaMeleeCore.h"
#include "RamaMeleeWeapon.h"
 
//~~~

//Apex issues if PhysX is included in Build.cs
/*
#if PLATFORM_ANDROID || PLATFORM_IOS
#ifdef WITH_APEX
#undef WITH_APEX
#endif
#define WITH_APEX 0
#endif //APEX EXCLUSIONS
*/

//UE5 PhysX / Joyful Chaos

//Life!!!! ♥ Rama
#include "PhysicsEngine/BodyInstance.h"

//Rama Joy Chaos Shape Include
#include "PhysicsEngine/PhysicsSettings.h"
//Rama Joy Chaos Shape Include

bool URamaMeleeCore::MeleeSweep(
	URamaMeleeWeapon* Comp,
	UWorld* World,
	AActor* ActorToIgnore,
	FHitResult& Hit,
	const FVector& Start,
	const FVector& End,
	const FRamaMeleeShape& RamaMeleeShape,
	const FCollisionObjectQueryParams& ObjectQueryParams,
	const FQuat& Rot
){
	if(!Comp || !World) return false;
	
	//Expose (not till multi threading solid tho)
	FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Sphere Sweep")), true, ActorToIgnore);
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(ActorToIgnore);
	
	//Physical Material
	TraceParams.bReturnPhysicalMaterial = true;
	  
	//Initial Overlaps
	TraceParams.bFindInitialOverlaps = Comp->HitEvents_IncludeInitialOverlapsDuringSwingStart;
	
	return World->SweepSingleByObjectType(
		Hit, 
		Start, 
		End, 
		Rot, 
		ObjectQueryParams, 
		RamaMeleeShape, 
		TraceParams
	);
}
 
bool URamaMeleeCore::MeleeSweep(
	URamaMeleeWeapon* Comp,
	UWorld* World,
	TArray<AActor*> ActorsToIgnore,
	FHitResult& Hit,
	const FVector& Start,
	const FVector& End,
	const FRamaMeleeShape& RamaMeleeShape,
	const FCollisionObjectQueryParams& ObjectQueryParams,
	const FQuat& Rot
){
	if(!Comp || !World) return false;
	
	//Expose (not till multi threading solid tho)
	FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Sphere Sweep")), true); //complex
	TraceParams.bTraceComplex = true;
	
	for(AActor* Each : ActorsToIgnore)
	{
		TraceParams.AddIgnoredActor(Each);
	}
	
	//Physical Material
	TraceParams.bReturnPhysicalMaterial = true;
	  
	//Initial Overlaps
	TraceParams.bFindInitialOverlaps = Comp->HitEvents_IncludeInitialOverlapsDuringSwingStart;
	
	return World->SweepSingleByObjectType(
		Hit, 
		Start, 
		End, 
		Rot, 
		ObjectQueryParams, 
		RamaMeleeShape, 
		TraceParams
	);
}
	
static void DrawBodyShapeIndex(UWorld* World, const FVector& Loc, int32 BodyIndex, int32 ShapeIndex)
{ 
	DrawDebugString(
		World, Loc, 
		FString::FromInt(BodyIndex) + ":" + FString::FromInt(ShapeIndex), 
		NULL, 						//class AActor* TestBaseActor = 
		FColor::Green, 
		0.f, 						// Duration
		true, 						//bool bDrawShadow
		3.333						//float FontScale = 1.f)
	);
}


static bool RamaHelperFunction_ChaosShapeIsValid(FPhysicsShapeHandle& Shape, const FPhysicsGeometryCollection& Geo, ECollisionShapeType GeoType)
{ 
	//Sometimes the Geo is invalid in ways that are beyond my control, so should NOT be checked / insta-crash
	// Usually during a weld operation / attach or SetSkeletalMesh
	 
	// Because I have no chance to determine if it is valid, because the IsValidGeometry() check afterward for geo is valid is AFTER the internal checked which I can't stop
	// ♥ Rama
	//Geom.GetObjectChecked<Chaos::TBox<Chaos::FReal,3>>();
				//! ~~~~~~~~
			
				
	if(!Shape.IsValid())
	{
		//! This is not stopping a crash on accessing the geometry during switches of the skeletal mesh, or ATTACHING / WELDING operations
		//! Epic should investigate
		//I am doing read-only operations, the geometry data itself is not valid at beginplay for some reason
		
		/*
		Assertion failed: T_DERIVED::StaticType() == Type [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Experimental\Chaos\Public\Chaos\ImplicitObject.h] [Line: 171]

		UnrealEditor_PhysicsCore
		UnrealEditor_RamaMeleePlugin!DrawBody_Internal() [D:\RamaPlugins\PMeleeWeapon\Plugins\RamaMeleeWeapon\Source\RamaMeleePlugin\Private\Core_PhysX.cpp:94]
		UnrealEditor_RamaMeleePlugin!<lambda_daccf33304b51ee28082313d890bfbfe>::operator()() [D:\RamaPlugins\PMeleeWeapon\Plugins\RamaMeleeWeapon\Source\RamaMeleePlugin\Private\Core_PhysX.cpp:179]
		UnrealEditor_PhysicsCore
		
		*/
				
		return false;
	}
	
	//!~~~~~~~~~~~~~~~~~~~~~~~
	//!~~~~~~~~~~~~~~~~~~~~~~~
	//!~~~~~~~~~~~~~~~~~~~~~~~
	// PERSISTENCE == VICTORY
	const Chaos::FImplicitObject& Attempt347 = Shape.GetGeometry();
	//!~~~~~~~~~~~~~~~~~~~~~~~
	//!~~~~~~~~~~~~~~~~~~~~~~~
	//!~~~~~~~~~~~~~~~~~~~~~~~
	
	if(GeoType == ECollisionShapeType::Sphere)
	{ 
		return Attempt347.GetObject<Chaos::TSphere<Chaos::FReal,3>>() != nullptr;
		//return Shape.GetObject<Chaos::TSphere<Chaos::FReal,3>>() != nullptr;
		//return Geo.GetObject<Chaos::TSphere<Chaos::FReal,3>>() != nullptr;
	}					///~~~~
	
	if(GeoType == ECollisionShapeType::Box)
	{
		return Attempt347.GetObject<Chaos::TBox<Chaos::FReal,3>>() != nullptr;
			 //Geom.GetObjectChecked<Chaos::TBox<Chaos::FReal,3>>();
		//return Geo.GetObject<Chaos::TBox<Chaos::FReal,3>>() != nullptr;
	}					///~~~~
	
	if(GeoType == ECollisionShapeType::Capsule)
	{ 
		return Attempt347.GetObject<Chaos::FCapsule>() != nullptr;
		//return Geo.GetObject<Chaos::FCapsule>() != nullptr;
	}					///~~~~
	
	return false;
	
}


//Draws with default shape color if not specified to draw red
// allows easy visualization of specific physx::PxShapes of the component!
static void DrawBody_Internal(
	URamaMeleeWeapon* Comp,
	UWorld* World, 
	int32 BodyIndex,
	int32 ShapeIndex,
	FPhysicsShapeHandle& Shape,
	const FTransform& ShapeGlobalTrans, 
	float Thickness,
	bool DrawRed,
	bool DrawIndex = false
){
	if(!Comp || !World)
	{
		return;
	}
	
	ECollisionShapeType GeoType = FPhysicsInterface::GetShapeType(Shape);
	FPhysicsGeometryCollection GeoCollection = FPhysicsInterface::GetGeometryCollection(Shape);
	
	//Rama Custom Crash Protection for Joyful Chaos Engine 
	if(!RamaHelperFunction_ChaosShapeIsValid(Shape, GeoCollection,GeoType))
	{
		//Strange bug in Chaos code, randomly, shapes go invalid, and seems to always happen on initial attach of weapon to character mesh
		Comp->RamaReportsChaosPhysicsShapesWentInvalidOnUs();
		
		return;
		//~~~~~
	}
		   
	//Sphere
	if(GeoType == ECollisionShapeType::Sphere)
	{
		auto JoySphere = GeoCollection.GetSphereGeometry();
		 
		//Validity Check! 
		// ♥ Rama
		if(!JoySphere.IsValidGeometry())
		{
			return;
		}
	
		FVector ShapeWorldCenter(ShapeGlobalTrans.TransformPosition(JoySphere.GetCenter()));
		
		if(DrawIndex)
		{
			DrawBodyShapeIndex(World,ShapeWorldCenter,BodyIndex,ShapeIndex);
		}
		
		DrawDebugSphere(
			World, 
			ShapeWorldCenter, 
			JoySphere.GetRadius(),
			24, 
			DrawRed ? FColor::Red : FColor::Yellow, 
			false, 
			-1.f //LifeTime 
		);
		return;
		//~~~~
	}
	 
	//Box
	if(GeoType == ECollisionShapeType::Box)
	{
		auto JoyBox = GeoCollection.GetBoxGeometry();
		  
		//Validity Check! 
		// ♥ Rama
		if(!JoyBox.IsValidGeometry())
		{
			return;
		}
		
		FVector ShapeWorldCenter(ShapeGlobalTrans.TransformPosition(JoyBox.GetCenter()));
		
		if(DrawIndex)
		{
			DrawBodyShapeIndex(World,ShapeWorldCenter,BodyIndex,ShapeIndex);
		}
		
		URamaMeleeCore::DrawBox(
			World, 
			ShapeWorldCenter, 
			JoyBox.Extents() * 0.5, 	//Box Half Extents
			ShapeGlobalTrans.GetRotation(), 
			DrawRed ? FColor::Red : FColor::Yellow, 
			Thickness	//Thickness!!!
		); 
		return;
		//~~~~
	}
	
	//Capsule
	if(GeoType == ECollisionShapeType::Capsule)
	{
		auto JoyCapsule = GeoCollection.GetCapsuleGeometry();
		 
		//Validity Check! 
		// ♥ Rama
		if(!JoyCapsule.IsValidGeometry())
		{
			return;
		}
		
		const float HalfHeight = JoyCapsule.GetHeight() * 0.5;
		    
		FVector CapsuleAxis(JoyCapsule.GetAxis());
		
		FVector ShapeWorldCenter(ShapeGlobalTrans.TransformPosition(JoyCapsule.GetCenter()));
		
		if(DrawIndex)
		{
			DrawBodyShapeIndex(World,ShapeWorldCenter,BodyIndex,ShapeIndex);
		}
		
		URamaMeleeCore::DrawCapsule(
			true, 	//PhysX mode so the FQuat rotation is drawn correctly for PhysX! 
			World, 
			ShapeWorldCenter, 
			HalfHeight, 
			JoyCapsule.GetRadius(), 
			ShapeGlobalTrans.GetRotation() * CapsuleAxis.ToOrientationQuat(), 
			DrawRed ? FColor::Red : FColor::Yellow, 
			false,  			//persistent
			-1.f, 				//Lifetime
			0, 				//depth
			Thickness/2
		); 
		return;
		//~~~~
	}
	
} 
static void DrawBody(URamaMeleeWeapon* Comp, FBodyInstance* Body, int32 BodyIndex, float Thickness)
{
	if(!Comp || !Comp->GetWorld() || !Body) 
	{
		return;
	}
		
	FTransform BodyWorldTrans 		= Body->GetUnrealWorldTransform();
	 
	TArray<FPhysicsShapeHandle> Shapes;
  
	FPhysicsActorHandle& ActorHandle = Body->GetPhysicsActorHandle();
	if(!ActorHandle)
	{
		return;
	}
	
	//I have to use a write lock because I call a function to recreate physics state if
	// after getting all the shapes, I detect the shapes have gone invalide.
	//My theory: I wonder if BodyInstance::GetAllShapes_AssumesLocked is somehow modifying the data it is retrieving
	FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle& Actor)
	{
		int32 ShapeCount = Body->GetAllShapes_AssumesLocked(Shapes);
		
		//UE_LOG(LogTemp,Warning,TEXT("Melee Weapon Body Index: %d, number of shapes : %d"), BodyIndex, Shapes.Num());
		 
		for(int32 ShapeIndex = 0; ShapeIndex < Shapes.Num(); ShapeIndex++)
		{
			FPhysicsShapeHandle& Shape = Shapes[ShapeIndex];
			   
			//Validity Check! ♥ Rama
			if(!Shape.IsValid()) 
			{
				continue;
			}
			
			// Pose of the shape in actor space
			FTransform ShapeLocalTrans = FPhysicsInterface::GetLocalTransform(Shape);
			FTransform ShapeWorldTrans = ShapeLocalTrans * BodyWorldTrans;
			   
			bool DrawRed = false;
			FRamaMeleeDamageInfo* EachBodyDamageInfo = Comp->DamageMap.GetDamageInfoForBodyIndex(BodyIndex);
			if(EachBodyDamageInfo)
			{
				DrawRed = EachBodyDamageInfo->ShapeIndicies.Contains(ShapeIndex);
			}	
			
			DrawBody_Internal(
				Comp,
				Comp->GetWorld(), 
				BodyIndex,
				ShapeIndex,
				Shape,
				ShapeWorldTrans, 
				Thickness,
				DrawRed,
				Comp->DrawShapeIndices
			);
		}
	});
}

void URamaMeleeCore::DrawRamaMeleeComp(URamaMeleeWeapon* Comp, float Thickness)
{
	if(!Comp|| ! Comp->GetWorld()) 
	{
		return;
	}
	
	//Part of Skeletal Mesh Component
	TArray<FBodyInstance*>& Bodies = Comp->Bodies;
   
	//For each body
	for(int32 v = 0; v < Bodies.Num(); v++)
	{
		FBodyInstance* EachBody = Bodies[v];
		 
		if(!EachBody || !EachBody->IsValidBodyInstance()) 
		{
			continue;
		}
		  
		DrawBody(Comp, EachBody, v, Thickness);
	}
}

void URamaMeleeCore::GetMeleeSweepData(
	URamaMeleeWeapon* RamaMeleeComp, 
	const TArray<FTransform>& StartBodyPose,
	TArray<FMeleeSweepData>& DamageShapes,
	FRamaMeleeDamageMap& DamageMap
){
	if(!RamaMeleeComp || !RamaMeleeComp->GetWorld()) 
	{
		return;
	}
	 
	//Part of Skeletal Mesh Component
	TArray<FBodyInstance*>& Bodies = RamaMeleeComp->Bodies;
   
	//Valid?
	if(Bodies.Num() < 1)
	{
		return;
	}
	 
	//For Each Body of Skeletal Mesh Weapon
	for(int32 b = 0; b < Bodies.Num(); b++)
	{ 
		FBodyInstance* Body = Bodies[b];
	
		//Valid?
		if(!Body->IsValidBodyInstance())
		{
			continue;
		}
		
		
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!! DECISION NOT TO SWEEP NON DAMAGING SHAPES YET IS RIGHT HERE
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		FRamaMeleeDamageInfo* EachBodyDamageInfo = DamageMap.GetDamageInfoForBodyIndex(b);
		if(!EachBodyDamageInfo)
		{
			//! Currently I only sweep damaging shapes
			//There have been requests to sweep non-damaging, will do that in a future update
			//once the basics are solid
			continue;
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//! <3 Rama
		}	
			
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		TArray<int32>& ShapeIndicies = EachBodyDamageInfo->ShapeIndicies;
	
		//Start and End Positions of Each Body
		
		//Saved from last tick in URamaMeleeWeapon::SwingTick()
		FTransform BodySweepStart 		= StartBodyPose[b];
		
		//Current
		FTransform BodySweepEnd 		= Body->GetUnrealWorldTransform();
		
		TArray<FPhysicsShapeHandle> Shapes;
		FPhysicsActorHandle& ActorHandle = Body->GetPhysicsActorHandle();
		if(!ActorHandle)
		{
			return;
		}
		   
		FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			int32 ShapeCount = Body->GetAllShapes_AssumesLocked(Shapes);
			for(int32 v = 0; v < Shapes.Num(); v++)
			{ 
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//!!! DECISION NOT TO SWEEP NON DAMAGING SHAPES YET IS RIGHT HERE
				//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				//Shape included as damaging?
				if(!ShapeIndicies.Contains(v))
				{ 
					continue;
					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					//! <3 Rama
				}
		
				FPhysicsShapeHandle& Shape = Shapes[v];
			
				ECollisionShapeType GeoType = FPhysicsInterface::GetShapeType(Shape);
				FPhysicsGeometryCollection GeoCollection = FPhysicsInterface::GetGeometryCollection(Shape);

				//Rama Custom Crash Protection for Joyful Chaos Engine 
				if(!RamaHelperFunction_ChaosShapeIsValid(Shape, GeoCollection,GeoType))
				{
					RamaMeleeComp->RamaReportsChaosPhysicsShapesWentInvalidOnUs();
					
					continue;
					//~~~~~
				}
				 
				// Pose of the shape in actor space
				FTransform ShapeLocalTrans = FPhysicsInterface::GetLocalTransform(Shape);
				
				FTransform ShapeStart 	= ShapeLocalTrans * BodySweepStart;
				FTransform ShapeEnd 	= ShapeLocalTrans * BodySweepEnd;
					
				//New Data
				DamageShapes.Add(FMeleeSweepData());
			
				//New Shape Data
				FMeleeSweepData& ShapeData = DamageShapes.Last();
				
				//Indices
				ShapeData.BodyIndex 	= b;
				ShapeData.ShapeIndex 	= v;
					
				//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
				//Update Rotation and Position to account for Chaos Shapes 
				// ♥ Rama
				//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
				//Sphere
				if(GeoType == ECollisionShapeType::Sphere)
				{
					auto JoySphere = GeoCollection.GetSphereGeometry();
					 
					FVector ShapeStartCenter(ShapeStart.TransformPosition(JoySphere.GetCenter()));
					FVector ShapeEndCenter(ShapeEnd.TransformPosition(JoySphere.GetCenter()));
					
					//Update!
					ShapeStart.SetTranslation(ShapeStartCenter);
					ShapeEnd.SetTranslation(ShapeEndCenter);
					
					//~~~ Finalize Shape Data! ~~~~
					//Global Shape Rotation
					ShapeData.Rotation = ShapeStart.GetRotation();
					
					//Start
					ShapeData.Start = ShapeStart.GetTranslation();
					
					//End
					ShapeData.End = ShapeEnd.GetTranslation();
			
					//Set Shape!
					ShapeData.SetSphere(JoySphere.GetRadius());
				}
				 
				//Box
				else if(GeoType == ECollisionShapeType::Box)
				{
					auto JoyBox = GeoCollection.GetBoxGeometry();
					  
					FVector ShapeStartCenter(ShapeStart.TransformPosition(JoyBox.GetCenter()));
					FVector ShapeEndCenter(ShapeEnd.TransformPosition(JoyBox.GetCenter()));
					
					//Update!
					ShapeStart.SetTranslation(ShapeStartCenter);
					ShapeEnd.SetTranslation(ShapeEndCenter);
						
					//~~~ Finalize Shape Data! ~~~~
					//Global Shape Rotation
					ShapeData.Rotation = ShapeStart.GetRotation();
					
					//Start
					ShapeData.Start = ShapeStart.GetTranslation();
					
					//End
					ShapeData.End = ShapeEnd.GetTranslation();
			
					//Set Shape!
					ShapeData.SetBox(FVector3f(JoyBox.Extents() * 0.5));
				}
				
				//Capsule
				else if(GeoType == ECollisionShapeType::Capsule)
				{
					auto JoyCapsule = GeoCollection.GetCapsuleGeometry();
					
					FVector ShapeStartCenter(ShapeStart.TransformPosition(JoyCapsule.GetCenter()));
					FVector ShapeEndCenter(ShapeEnd.TransformPosition(JoyCapsule.GetCenter()));
					
					//Update!
					ShapeStart.SetTranslation(ShapeStartCenter);
					ShapeEnd.SetTranslation(ShapeEndCenter);
					
					const float HalfHeight = JoyCapsule.GetHeight() * 0.5;
						
					//Capsule Rotation!
					//  ♥ Rama
					FVector CapsuleAxis(JoyCapsule.GetAxis());
					
					//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
					//Rama Magics for You!
					// Honoring the rotation of each body relative to component, the world space rotation 
					// of the component, in motion of animation in the world, 
					// And also the ~ capsule axis ~ that is relative to the physics body asset
					// ♥ Rama
					//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
					FQuat JoyRot = ShapeStart.GetRotation() * CapsuleAxis.ToOrientationQuat();
					//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
					
					//Update Rotation for Capsule 
					// ♥ Rama
					ShapeStart.SetRotation(JoyRot);
					
					//ShapeEnd.SetRotation(JoyRot);
					
					//~~~ Finalize Shape Data! ~~~~
					//Global Shape Rotation
					ShapeData.Rotation = ShapeStart.GetRotation();
					
					//Start
					ShapeData.Start = ShapeStart.GetTranslation();
					 
					//End
					ShapeData.End = ShapeEnd.GetTranslation();
			 
					//Set Shape!
					ShapeData.SetCapsule(JoyCapsule.GetRadius(),HalfHeight);
				}
				else
				{
					//Convex or trimesh or heightmap
					  
					//Invalid data, I am not setting the shape data
					DamageShapes.Pop();
					
					continue;
				}
				//♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥♥
			}
		});		
	}
}
