#include "LoadingScreenFunctionLibrary.h"
#include "SLoadingScreenStart.h"
#include "SLoadingScreenGame.h"
#include "Widgets/SWeakWidget.h"
#include "LoadingScreenSettings.h"
#include "DelayAction.h"

int32 ULoadingScreenFunctionLibrary::DisplayLoadIndex = -1;

void ULoadingScreenFunctionLibrary::SetDisplayLoadIndex(int32 NewLoadIndex)
{
	ULoadingScreenFunctionLibrary::DisplayLoadIndex = NewLoadIndex;
}

bool ULoadingScreenFunctionLibrary::IsInEditor()
{
	return GIsEditor;
}

void ULoadingScreenFunctionLibrary::DebugDeleteAllWidgets()
{
	GEngine->GameViewport->RemoveAllViewportWidgets();
}

bool ULoadingScreenFunctionLibrary::DebugGenerateStartScreen()
{
	//this should work
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	TSharedPtr<SLoadingScreenStart> StartScreen = SNew(SLoadingScreenStart, Settings->StartupLoadingScreen);

	GEngine->GameViewport->AddViewportWidgetContent(
		StartScreen.ToSharedRef(), 1
		//.PossiblyNullContent(StartScreen.ToSharedRef())
	);

	return StartScreen.IsValid();
}

bool ULoadingScreenFunctionLibrary::DebugGenerateLoadScreen()
{
	//this should work
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	TSharedPtr<SLoadingScreenGame> LoadScreen = SNew(SLoadingScreenGame, Settings->DefaultLoadingScreen);

	//the 1 here is zorder. Remember that
	GEngine->GameViewport->AddViewportWidgetContent(
		LoadScreen.ToSharedRef(), 1
		//.PossiblyNullContent(StartScreen.ToSharedRef())
	);

	return LoadScreen.IsValid();
}

bool ULoadingScreenFunctionLibrary::MakeLoadingScreen(const UObject* WorldContextObject, struct FLatentActionInfo LatentInfo, int32 ZOrder)
{
	//First create a loading screen
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	TSharedPtr<SLoadingScreenGame> LoadScreen = SNew(SLoadingScreenGame, Settings->DefaultLoadingScreen);

	GEngine->GameViewport->AddViewportWidgetContent(
		LoadScreen.ToSharedRef(), ZOrder
	);

	//This is the delay by one tick function from KismetSystemLibrary. We want this for blueprint flow; so the execution pin is called when the UI is actually visible.
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FDelayUntilNextTickAction(LatentInfo));
		}
	}

	////if an actual delay turns out to be better...
	//if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	//{
	//	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	//	if (LatentActionManager.FindExistingAction<FDelayAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
	//	{
	//		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FDelayAction(Duration, LatentInfo));
	//	}
	//}

	return LoadScreen.IsValid();
}