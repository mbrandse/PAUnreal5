#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadingScreenFunctionLibrary.generated.h"

UCLASS()
class PIRATESADVENTUREUE5LOADINGSCREEN_API ULoadingScreenFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	static int32 DisplayLoadIndex;

public:

	UFUNCTION(BlueprintCallable, Category = "Loading")
		static void SetDisplayLoadIndex(int32 NewLoadTextIndex);

	static inline int32 GetDisplayLoadIndex() { return DisplayLoadIndex; }

	//Checkers
	/** Returns true if this is being run from an editor preview */
	UFUNCTION(BlueprintPure, Category = "Loading")
		static bool IsInEditor();

	//Debugging functions
	//Delete all widgets. Usable for testing the start screen and loading screen
	UFUNCTION(BlueprintCallable, Category = "Loading Debug")
		static void DebugDeleteAllWidgets();

	//Attempts to generate the start screen. This function is purely meant to test out the layout and has nothing to do with the loading screen.
	UFUNCTION(BlueprintCallable, Category = "Loading Debug")
		static bool DebugGenerateStartScreen();

	//Attempts to generate the in-game load screen. This function is purely meant to test out the layout and has nothing to do with the loading screen.
	UFUNCTION(BlueprintCallable, Category = "Loading Debug")
		static bool DebugGenerateLoadScreen();

	UFUNCTION(BlueprintCallable, Category = "Loading Screen", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "LatentInfo"))
		static bool	MakeLoadingScreen(const UObject* WorldContextObject, struct FLatentActionInfo LatentInfo, int32 ZOrder = 0);
};
