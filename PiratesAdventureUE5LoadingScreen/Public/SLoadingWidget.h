#pragma once

#include "Widgets/SCompoundWidget.h"
#include "LoadingScreenSettings.h"

class FDeferredCleanupSlateBrush;
struct FLoadingWidgetSettings;

/**
 * Loading Widget base class
 */
class SLoadingWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingWidget) {}

	SLATE_END_ARGS()

	// SWidgetOverrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** Construct loading icon*/
	void ConstructLoadingIcon(const FLoadingWidgetSettings& Settings);

	//the actual construct
	void Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings);

protected:
	// Placeholder widgets
	TSharedRef<SWidget> LoadingIcon = SNullWidget::NullWidget;
	// Image slate brush list
	TArray<TSharedPtr<FDeferredCleanupSlateBrush>> CleanupBrushList;	

	// Current image sequence index
	mutable int32 ImageIndex = 0;

	// Current total delta time
	mutable float TotalDeltaTime = 0.0f;

	//Time in second to update the images, the smaller value the faster of the animation. A zero value will update the images every frame.
	float Interval = 0.05f;	
	float FinalInterval = 0.05f;
	mutable float CurrentInterval;
	
	// Getter for text visibility
	EVisibility GetLoadingWidgetVisibility() const;	
};
