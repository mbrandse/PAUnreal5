// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
using UnrealBuildTool;

public class RamaSaveSystem : ModuleRules
{ 
	//previous versions won't package with the new setup, so I'd rather get the warning in 4.16
	//then have 4 previous batch builds fail <3 Rama
	//public RamaSaveSystem(TargetInfo Target)
	public RamaSaveSystem(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "Private/RamaSaveSystemPrivatePCH.h";

        PCHUsage = PCHUsageMode.UseSharedPCHs;
		
		bLegacyPublicIncludePaths = false;
		
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"DeveloperSettings"
			}
		);
	}
}
