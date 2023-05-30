// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PiratesAdventureUE5 : ModuleRules
{
	public PiratesAdventureUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"Niagara",
			"RamaMeleePlugin",
			"RamaSaveSystem",
			"DialoguePlugin",
			"Slate",
			"SlateCore",
			"UMG",
            "AIModule",
			"CinematicCamera",
			"MoviePlayer",
            "DeveloperSettings",
			"PiratesAdventureUE5LoadingScreen"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
            "UMG",
            "MoviePlayer",
			"DeveloperSettings",
			"PiratesAdventureUE5LoadingScreen"
		});
	}
}
