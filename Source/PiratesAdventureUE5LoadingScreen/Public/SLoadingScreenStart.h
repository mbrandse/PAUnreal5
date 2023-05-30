#pragma once

#include "SLoadingScreenBase.h"

struct FPiratesAdventureLoadingScreenSettings;
class FDeferredCleanupSlateBrush;

/**
 * Classic layout loading screen
 */
class SLoadingScreenStart : public SLoadingScreenBase
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreenStart) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	 */
	void Construct(const FArguments& InArgs, const FPiratesAdventureLoadingScreenSettings& Settings);

private:
	TSharedPtr<FDeferredCleanupSlateBrush> HeaderImageBrush;
};
