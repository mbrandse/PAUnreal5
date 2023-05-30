// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
//#include "LoadingScreenSettings.h"

DECLARE_LOG_CATEGORY_EXTERN(PALoadScreen, All, All)

class FPiratesAdventureUE5LoadingScreenModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override;

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PiratesAdventureUE5LoadingScreen");
	}

	static inline FPiratesAdventureUE5LoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPiratesAdventureUE5LoadingScreenModule>("PiratesAdventureUE5LoadingScreen");
	}
};

//All this shit is old; remove the moment we get the loading screen working.
////struct FPiratesAdventureLoadingScreenSettings;
//
///** Module interface for this game's loading screens */
//class FPiratesAdventureUE5LoadingScreenModule : public IModuleInterface
//{
//public:
//	/** IModuleInterface implementation */
//	/**
//	 * Called right after the module DLL has been loaded and the module object has been created
//	 */
//	virtual void StartupModule() override;
//
//	/**
//	 * Called before the module is unloaded, right before the module object is destroyed.
//	 */
//	virtual void ShutdownModule() override;
//
//	/**
//	 * Returns true if this module hosts gameplay code
//	 *
//	 * @return True for "gameplay modules", or false for engine code modules, plugins, etc.
//	 */
//	virtual bool IsGameModule() const override;
//
//	/**
//	 * Singleton-like access to this module's interface. This is just for convenience!
//	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
//	 *
//	 * @return Returns singleton instance, loading the module on demand if needed
//	 */
//	static inline FPiratesAdventureUE5LoadingScreenModule& Get()
//	{
//		return FModuleManager::LoadModuleChecked<FPiratesAdventureUE5LoadingScreenModule>("PiratesAdventureUE5LoadingScreen");
//	}
//
//	/**
//	 * Checks to see if this module is loaded and ready. It is only valid to call Get() if IsAvailable() returns true.
//	 *
//	 * @return True if the module is loaded and ready to use
//	 */
//	static inline bool IsAvailable()
//	{
//		return FModuleManager::Get().IsModuleLoaded("PiratesAdventureUE5LoadingScreen");
//	}
//
//private:
//	/**
//	 * Loading screen callback, it won't be called if we've already explicitly setup the loading screen
//	 */
//	void PreSetupLoadingScreen();
//
//	/**
//	 * Setup loading screen settings
//	 */
//	void SetupLoadingScreen(const FPiratesAdventureLoadingScreenSettings& LoadingScreenSettings);
//
//public:
//
//	///** Loads the module so it can be turned on */
//	//static inline FPiratesAdventureUE5LoadingScreenModule& Get()
//	//{
//	//	return FModuleManager::LoadModuleChecked<FPiratesAdventureUE5LoadingScreenModule>("PiratesAdventureUE5LoadingScreen");
//	//}
//
//	//THIS STILL CRASHES THE ENGINE HORRIBLY!
//	/** Kicks off the loading screen for in game loading (not startup) */
//	virtual void StartInGameLoadingScreen(class UUserWidget *Widget, bool bPlayUntilStopped, float PlayTime) = 0;
//
//	/** Stops the loading screen */
//	virtual void StopInGameLoadingScreen() = 0;
//};
