// Copyright Epic Games, Inc. All Rights Reserved.

#include "PiratesAdventureUE5LoadingScreen.h"
#include "Modules/ModuleManager.h"
//#include "SlateBasics.h"
//#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "LoadingScreenSettings.h"
//#include "Widgets/Images/SThrobber.h"
//#include "Blueprint/UserWidget.h"
//#include "Widgets/SWidget.h"
//#include "Components/Widget.h"

IMPLEMENT_GAME_MODULE(FPiratesAdventureUE5LoadingScreenModule, PiratesAdventureUE5LoadingScreen);

DEFINE_LOG_CATEGORY(PALoadScreen)

#define LOCTEXT_NAMESPACE "PiratesAdventureUE5LoadingScreen"

void FPiratesAdventureUE5LoadingScreenModule::StartupModule()
{
	UE_LOG(PALoadScreen, Warning, TEXT("PiratesAdventureLoadScreen: Log Started"));
}

void FPiratesAdventureUE5LoadingScreenModule::ShutdownModule()
{
	UE_LOG(PALoadScreen, Warning, TEXT("PiratesAdventureLoadScreen: Log Ended"));
}

bool FPiratesAdventureUE5LoadingScreenModule::IsGameModule() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE

//#define LOCTEXT_NAMESPACE "FPiratesAdventureUE5LoadingScreenModule"
//
//void FPiratesAdventureUE5LoadingScreenModule::StartupModule()
//{
//	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
//
//
//	if (!IsRunningDedicatedServer() && FSlateApplication::IsInitialized())
//	{
//		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
//
//		if (IsMoviePlayerEnabled())
//		{
//			GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, &FPiratesAdventureUE5LoadingScreenModule::PreSetupLoadingScreen);
//		}
//
//		// Prepare the startup screen, the PreSetupLoadingScreen callback won't be called
//		// if we've already explicitly setup the loading screen
//		SetupLoadingScreen(Settings->StartupLoadingScreen);
//	}
//}
//
//void FPiratesAdventureUE5LoadingScreenModule::ShutdownModule()
//{
//	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
//	// we call this function before unloading the module.
//
//	if (!IsRunningDedicatedServer())
//	{
//		// TODO: Unregister later
//		GetMoviePlayer()->OnPrepareLoadingScreen().RemoveAll(this);
//	}
//}
//
//bool FPiratesAdventureUE5LoadingScreenModule::IsGameModule() const
//{
//	return true;
//}
//
//void FPiratesAdventureUE5LoadingScreenModule::PreSetupLoadingScreen()
//{
//	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
//	SetupLoadingScreen(Settings->DefaultLoadingScreen);
//}
//
//void FPiratesAdventureUE5LoadingScreenModule::SetupLoadingScreen(const FPiratesAdventureLoadingScreenSettings& LoadingScreenSettings)
//{
//	TArray<FString> MoviesList = LoadingScreenSettings.MoviePaths;
//
//	if (LoadingScreenSettings.bSetDisplayMovieIndexManually == true)
//	{
//		MoviesList.Empty();
//
//		// Show specific movie if valid otherwise show original movies list
//		if (LoadingScreenSettings.MoviePaths.IsValidIndex(ULoadingScreenLibrary::GetDisplayMovieIndex()))
//		{
//			MoviesList.Add(LoadingScreenSettings.MoviePaths[ULoadingScreenLibrary::GetDisplayMovieIndex()]);
//		}
//		else
//		{
//			MoviesList = LoadingScreenSettings.MoviePaths;
//		}
//	}
//
//	FLoadingScreenAttributes LoadingScreen;
//	LoadingScreen.MinimumLoadingScreenDisplayTime = LoadingScreenSettings.MinimumLoadingScreenDisplayTime;
//	LoadingScreen.bAutoCompleteWhenLoadingCompletes = LoadingScreenSettings.bAutoCompleteWhenLoadingCompletes;
//	LoadingScreen.bMoviesAreSkippable = LoadingScreenSettings.bMoviesAreSkippable;
//	LoadingScreen.bWaitForManualStop = LoadingScreenSettings.bWaitForManualStop;
//	LoadingScreen.bAllowInEarlyStartup = LoadingScreenSettings.bAllowInEarlyStartup;
//	LoadingScreen.bAllowEngineTick = LoadingScreenSettings.bAllowEngineTick;
//	LoadingScreen.MoviePaths = MoviesList;
//	LoadingScreen.PlaybackType = LoadingScreenSettings.PlaybackType;
//
//	if (LoadingScreenSettings.bShowWidgetOverlay)
//	{
//		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
//
//		//switch (LoadingScreenSettings.Layout)
//		//{
//		//case EAsyncLoadingScreenLayout::ALSL_Classic:
//		//	LoadingScreen.WidgetLoadingScreen = SNew(SClassicLayout, LoadingScreenSettings, Settings->Classic);
//		//	break;
//		//case EAsyncLoadingScreenLayout::ALSL_Center:
//		//	LoadingScreen.WidgetLoadingScreen = SNew(SCenterLayout, LoadingScreenSettings, Settings->Center);
//		//	break;
//		//case EAsyncLoadingScreenLayout::ALSL_Letterbox:
//		//	LoadingScreen.WidgetLoadingScreen = SNew(SLetterboxLayout, LoadingScreenSettings, Settings->Letterbox);
//		//	break;
//		//case EAsyncLoadingScreenLayout::ALSL_Sidebar:
//		//	LoadingScreen.WidgetLoadingScreen = SNew(SSidebarLayout, LoadingScreenSettings, Settings->Sidebar);
//		//	break;
//		//case EAsyncLoadingScreenLayout::ALSL_DualSidebar:
//		//	LoadingScreen.WidgetLoadingScreen = SNew(SDualSidebarLayout, LoadingScreenSettings, Settings->DualSidebar);
//		//	break;
//		//}
//
//	}
//
//	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
//}
//
//#undef LOCTEXT_NAMESPACE
//
//IMPLEMENT_GAME_MODULE(FPiratesAdventureUE5LoadingScreenModule, PiratesAdventureUE5LoadingScreen);

//// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!
//struct FPiratesAdventureLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
//{
//	FPiratesAdventureLoadingScreenBrush(const FName InTextureName, const FVector2D& InImageSize)
//		: FSlateDynamicImageBrush(InTextureName, InImageSize)
//	{
//		SetResourceObject(LoadObject<UObject>(NULL, *InTextureName.ToString()));
//	}
//
//	virtual void AddReferencedObjects(FReferenceCollector& Collector)
//	{
//		if (UObject* CachedResourceObject = GetResourceObject())
//		{
//			Collector.AddReferencedObject(CachedResourceObject);
//		}
//	}
//};
//
//class SPiratesAdventureLoadingScreen : public SCompoundWidget
//{
//public:
//	SLATE_BEGIN_ARGS(SPiratesAdventureLoadingScreen) {}
//	SLATE_END_ARGS()
//
//	void Construct(const FArguments& InArgs)
//	{
//		// Load version of the logo with text baked in, path is hardcoded because this loads very early in startup
//		static const FName LoadingScreenName(TEXT("/Game/UI/Tex/T_StartScreen.T_StartScreen"));
//
//		LoadingScreenBrush = MakeShareable(new FPiratesAdventureLoadingScreenBrush(LoadingScreenName, FVector2D(1024, 128)));
//		
//		FSlateBrush *BGBrush = new FSlateBrush();
//		BGBrush->TintColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
//
//		ChildSlot
//			[
//			SNew(SOverlay)
//			+ SOverlay::Slot()
//			.HAlign(HAlign_Fill)
//			.VAlign(VAlign_Fill)
//			[
//				SNew(SBorder)	
//				.BorderImage(BGBrush)
//			]
//			+SOverlay::Slot()
//			.HAlign(HAlign_Center)
//			.VAlign(VAlign_Center)
//			[
//				SNew(SImage)
//				.Image(LoadingScreenBrush.Get())
//			]
//			+SOverlay::Slot()
//			.HAlign(HAlign_Fill)
//			.VAlign(VAlign_Fill)
//			[
//				SNew(SVerticalBox)
//				+SVerticalBox::Slot()
//				.VAlign(VAlign_Bottom)
//				.HAlign(HAlign_Right)
//				.Padding(FMargin(10.0f))
//				[
//					SNew(SThrobber)
//					.Visibility(this, &SPiratesAdventureLoadingScreen::GetLoadIndicatorVisibility)
//				]
//			]
//		];
//	}
//
//private:
//	/** Rather to show the ... indicator */
//	EVisibility GetLoadIndicatorVisibility() const
//	{
//		bool Vis =  GetMoviePlayer()->IsLoadingFinished();
//		return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Collapsed : EVisibility::Visible;
//	}
//	
//	/** Loading screen image brush */
//	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
//};
//
//class FPiratesAdventureUE5LoadingScreenModule : public FPiratesAdventureUE5LoadingScreenModule
//{
//public:
//	virtual void StartupModule() override
//	{
//		// Force load for cooker reference
//		LoadObject<UObject>(nullptr, TEXT("/Game/UI/Tex/T_StartScreen.T_StartScreen") );
//
//		if (IsMoviePlayerEnabled())
//		{
//			CreateScreen();
//		}
//	}
//	
//	virtual bool IsGameModule() const override
//	{
//		return true;
//	}
//
//	virtual void StartInGameLoadingScreen(UUserWidget *Widget, bool bPlayUntilStopped, float PlayTime) override
//	{
//		FLoadingScreenAttributes LoadingScreen;
//		LoadingScreen.bAutoCompleteWhenLoadingCompletes = !bPlayUntilStopped;
//		LoadingScreen.bWaitForManualStop = bPlayUntilStopped;
//		LoadingScreen.bAllowEngineTick = bPlayUntilStopped;
//		LoadingScreen.MinimumLoadingScreenDisplayTime = PlayTime;
//		//LoadingScreen.WidgetLoadingScreen = Widget->TakeWidget();
//		LoadingScreen.WidgetLoadingScreen = SNew(SPiratesAdventureLoadingScreen);
//		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
//	}
//
//	virtual void StopInGameLoadingScreen() override
//	{
//		GetMoviePlayer()->StopMovie(); 
//	}
//
//	virtual void CreateScreen()
//	{
//		//does this create a temporary screen I wonder? Perhaps it's a temp one before the actual one pops up. Test it.
//		FLoadingScreenAttributes LoadingScreen;
//		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
//		LoadingScreen.MinimumLoadingScreenDisplayTime = 3.f;
//		LoadingScreen.WidgetLoadingScreen = SNew(SPiratesAdventureLoadingScreen);
//		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
//	}
//
//};
//
//IMPLEMENT_GAME_MODULE(FPiratesAdventureUE5LoadingScreenModule, PiratesAdventureUE5LoadingScreen);
