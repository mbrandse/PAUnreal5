// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

public class PiratesAdventureUE5LoadingScreen : ModuleRules
{
	public PiratesAdventureUE5LoadingScreen(ReadOnlyTargetRules Target)
		: base(Target)
	{
		PrivatePCHHeaderFile = "Public/PiratesAdventureUE5LoadingScreen.h";

		PCHUsage = PCHUsageMode.UseSharedPCHs;

		PrivateIncludePaths.Add("PiratesAdventureUE5LoadingScreen/Private");

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
                "MoviePlayer",
                "Slate",
                "SlateCore",
                "UMG",
                "DeveloperSettings"
            }
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"MoviePlayer",
				"Slate",
				"SlateCore",
				"InputCore",
                "UMG",
                "DeveloperSettings"
            }
		);
	}
}
