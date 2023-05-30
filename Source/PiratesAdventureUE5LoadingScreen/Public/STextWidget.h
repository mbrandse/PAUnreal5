#pragma once

#include "Widgets/SCompoundWidget.h"

struct FLoadTextSettings;
class FDeferredCleanupSlateBrush;

class STextWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextWidget) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FLoadTextSettings& Settings);

private:
	TSharedPtr<FDeferredCleanupSlateBrush> LoadImageBrush;
};
