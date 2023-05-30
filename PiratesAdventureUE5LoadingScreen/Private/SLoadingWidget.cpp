#include "SLoadingWidget.h"
#include "Widgets/Images/SImage.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Texture2D.h"
#include "MoviePlayer.h"
#include "Widgets/SCompoundWidget.h"
#include "LoadingScreenFunctionLibrary.h"

void SLoadingWidget::Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings)
{
	// Root is a Horizontal Box
	TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);

	// Construct Loading Icon Widget
	ConstructLoadingIcon(Settings);

	//What we want here is ------loading icon - space - text. Everything outlined from the right. 
	// Add Loading Icon on the left first
	if(Settings.bShowLoadingText)
	{
		Root.Get().AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					LoadingIcon
				]

				//Add a spacer between text and icon
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					SNew(SSpacer)
					.Size(FVector2D(Settings.LoadingWidgetSideMargin, 0.0f))
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.AutoWidth()
				[
					SNew(STextBlock)
					.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
					.Font(Settings.Appearance.Font)
					.ShadowOffset(Settings.Appearance.ShadowOffset)
					.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
					.Justification(Settings.Appearance.Justification)
					.Text(Settings.LoadingText)
					.Margin(FMargin(0.f, 0.f, 0.f, Settings.LoadingTextBottomMargin))
				]

				//Add a spacer to the right of the text too
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				.AutoWidth()
				[
					SNew(SSpacer)
					.Size(FVector2D(Settings.LoadingTextSideMargin, 0.0f))
				]

			];

		// Add a bottom spacer
		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(FVector2D(0.f, Settings.LoadingWidgetBottomMargin))
			];
	}
	else
	{
		//if no loading text, just add the icon with the bottom/right margin. 
		//first test if current implement works.
		Root.Get().AddSlot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				LoadingIcon
			];
	}

	ChildSlot
		[
			Root
		];
}

int32 SLoadingWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{		
	TotalDeltaTime += Args.GetDeltaTime();

	//float CurrentInterval = ImageIndex == CleanupBrushList.Num() -1 ? FinalInterval : Interval; //set the interval based on the frame number

	if (TotalDeltaTime >= CurrentInterval)
	{
		if (CleanupBrushList.Num() > 1)
		{
			ImageIndex++;

			if (ImageIndex >= CleanupBrushList.Num())
			{
				ImageIndex = 0;
			}

			CurrentInterval = Interval;
			if(ImageIndex == CleanupBrushList.Num() - 1)
				CurrentInterval = FinalInterval;

			StaticCastSharedRef<SImage>(LoadingIcon)->SetImage(CleanupBrushList[ImageIndex].IsValid() ? CleanupBrushList[ImageIndex]->GetSlateBrush() : nullptr);			
		}

		TotalDeltaTime = 0.0f;
	}
	

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SLoadingWidget::ConstructLoadingIcon(const FLoadingWidgetSettings& Settings)
{
	int32 SequenceIndex = 0;
	if (Settings.ImageSequenceSettings.Num() > 1)
	{
		SequenceIndex = FMath::RandRange(0, Settings.ImageSequenceSettings.Num() - 1);
	}

	if (Settings.ImageSequenceSettings.IsValidIndex(SequenceIndex))
	{
		//int32 SequenceIndex = ULoadingScreenFunctionLibrary::GetDisplayLoadImageIndex();

		// Loading Widget is image sequence
		if (Settings.ImageSequenceSettings[SequenceIndex].Images.Num() > 0)
		{
			Interval = Settings.ImageSequenceSettings[SequenceIndex].Interval;
			FinalInterval = Settings.ImageSequenceSettings[SequenceIndex].FinalInterval;
			CurrentInterval = Interval;
			CleanupBrushList.Empty();
			ImageIndex = 0;

			FVector2D Scale = Settings.ImageSequenceSettings[SequenceIndex].Scale;

			for (auto Image: Settings.ImageSequenceSettings[SequenceIndex].Images)
			{
				if (Image)
				{
					CleanupBrushList.Add(FDeferredCleanupSlateBrush::CreateBrush(Image, FVector2D(Image->GetSurfaceWidth() * Scale.X, Image->GetSurfaceHeight() * Scale.Y)));					
				}				
			}
		
			// Create Image slate widget
			LoadingIcon = SNew(SImage)
				.Image(CleanupBrushList[ImageIndex]->GetSlateBrush());

			//// Set the first image index; but keep updating it in the paint.
			//Interval = Settings.ImageSequenceSettings[ImageIndex].Interval;
			//FinalInterval = Settings.ImageSequenceSettings[ImageIndex].FinalInterval;
		}
		else
		{
			// If there is no image in the array then create a spacer instead
			LoadingIcon = SNew(SSpacer).Size(FVector2D::ZeroVector);
		}
	}

	// Set Loading Icon render transform
	LoadingIcon.Get().SetRenderTransform(FSlateRenderTransform(FScale2D(Settings.TransformScale), Settings.TransformTranslation));
	LoadingIcon.Get().SetRenderTransformPivot(Settings.TransformPivot);

	// Hide loading widget when level loading is done if bHideLoadingWidgetWhenCompletes is true 
	if (Settings.bHideLoadingWidgetWhenCompletes)
	{		
		SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateRaw(this, &SLoadingWidget::GetLoadingWidgetVisibility)));
	}
}

EVisibility SLoadingWidget::GetLoadingWidgetVisibility() const
{
	return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Hidden : EVisibility::Visible;
}