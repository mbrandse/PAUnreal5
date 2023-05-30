// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
using UnrealBuildTool;

public class RamaMeleePlugin : ModuleRules
{
	public RamaMeleePlugin(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "Private/RamaMeleePluginPrivatePCH.h";

        PCHUsage = PCHUsageMode.UseSharedPCHs;
		
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"PhysicsCore", 
                //"PhysX", 		//hushed gasp, hands to mouth, is it truly possible? ♥ Rama
				"Chaos",  //<<~~~~~~~~~~~~~~~~~~
				"DeveloperSettings"
			}
		);
		 
		//APEX EXCLUSIONS
		if (Target.Platform != UnrealTargetPlatform.Android && Target.Platform != UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"APEX"
			}
			);
		}
	}
	 
	
}
