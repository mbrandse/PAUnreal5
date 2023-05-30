#pragma once

#include "SLoadingScreenBase.h"

struct FPiratesAdventureLoadingScreenSettings;
class FDeferredCleanupSlateBrush;

class SLoadingScreenGame : public SLoadingScreenBase
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreenGame) {}

	SLATE_END_ARGS()

	/**
	 * Construct this widget
	*/
	void Construct(const FArguments& InArgs, const FPiratesAdventureLoadingScreenSettings& Settings);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TSharedPtr<FDeferredCleanupSlateBrush> BGBrush;
	TSharedPtr<FDeferredCleanupSlateBrush> LoadingImageBrush;
	mutable bool bIsVisible = false;
	mutable int32 TickCount = 0;

	EVisibility GetUIVisibility() const
	{
		return bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
	};
};
