#include "SLoadingScreenStart.h"
#include "LoadingScreenSettings.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SSpacer.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Engine/Texture2D.h"
#include "SLoadingWidget.h"
#include "PiratesAdventureUE5LoadingScreen.h"

void SLoadingScreenStart::Construct(const FArguments& InArgs, const FPiratesAdventureLoadingScreenSettings& Settings)
{
	// First setup Root widget and a basic background (the startscreen is super basic)
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			//first create a super basic background color.
			SNew(SImage)
			.ColorAndOpacity(Settings.Background.BackgroundColor)
		];

	//Try to get start screen image. 
	const FSoftObjectPath& HeaderImageAsset = Settings.Background.BackgroundImage;
	UObject* HeaderImageObject = HeaderImageAsset.TryLoad();
	if (UTexture2D* HeaderImage = Cast<UTexture2D>(HeaderImageObject))
	{
		//For some reason using SurfaceHeight doesn't work here. Perhaps because the variable comes from a softobjectpath, rather than a direct Texture reference
		HeaderImageBrush = FDeferredCleanupSlateBrush::CreateBrush(HeaderImage, Settings.Background.BackgroundSize);

		//since this is the start screen, we will assume by default that we need no stretching or tiling. Just the image put on once. 
		Root.Get().AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				//stick the header in.
				SNew(SImage)
				.Image(HeaderImageBrush.IsValid() ? HeaderImageBrush->GetSlateBrush() : nullptr)
			]
		];
	}
	
	//make a horizontal box for the loader graphic, make the loader as well
	TSharedRef<SVerticalBox> HorizontalBox = SNew(SVerticalBox);
	TSharedRef<SLoadingWidget> LoadingWidget = SNew(SLoadingWidget, Settings.LoadingWidget);

	HorizontalBox.Get().AddSlot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.AutoHeight()
		[
			LoadingWidget
		];

	//Finally add the whole thing to root to finish off the start screen.
	Root->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		[
			SNew(SSafeZone)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.IsTitleSafe(true)
			[
				SNew(SDPIScaler)
				.DPIScale(this, &SLoadingScreenStart::GetDPIScale)
				[
					HorizontalBox
				]
			]
		];

	ChildSlot
		[
			Root
		];

	//I believe the spacer is basically used to fill up any empty space. 
	//The horizontal box is a box that fills horizontally, or something?

	//Also, try to use this to test this widget in-game as a non-loading screen (so we can test if the layout works properly)
	/*
		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget)
			.PossiblyNullContent(MyWidgetClass) //is this necessary? Don't think so.
		);
	*/

	//// Placeholder for loading widget
	//TSharedRef<SWidget> LoadingWidget = SNullWidget::NullWidget;
	//if (Settings.LoadingWidget.LoadingWidgetType == ELoadingWidgetType::LWT_Horizontal)
	//{
	//	LoadingWidget = SNew(SHorizontalLoadingWidget, Settings.LoadingWidget);
	//}
	//else
	//{
	//	LoadingWidget = SNew(SVerticalLoadingWidget, Settings.LoadingWidget);
	//}

	//TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	//if (LayoutSettings.bIsLoadingWidgetAtLeft)
	//{
	//	// Add Loading widget on left first
	//	HorizontalBox.Get().AddSlot()
	//		.VAlign(VAlign_Center)
	//		.HAlign(HAlign_Center)
	//		.AutoWidth()
	//		[
	//			LoadingWidget
	//		];

	//	// Add spacer at midder
	//	HorizontalBox.Get().AddSlot()
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		.AutoWidth()
	//		[
	//			SNew(SSpacer)
	//			.Size(FVector2D(LayoutSettings.Space, 0.0f))
	//		];

	//		// Tip Text on the right
	//	HorizontalBox.Get().AddSlot()
	//		.FillWidth(1.0f)
	//		.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
	//		.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
	//		[
	//			SNew(STipWidget, Settings.TipWidget)
	//		];
	//}
	//else
	//{
	//	// Tip Text on the left
	//	HorizontalBox.Get().AddSlot()
	//		.FillWidth(1.0f)
	//		.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
	//		.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
	//		[
	//			// Add tip text
	//			SNew(STipWidget, Settings.TipWidget)
	//		];

	//	// Add spacer at midder
	//	HorizontalBox.Get().AddSlot()
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		.AutoWidth()
	//		[
	//			SNew(SSpacer)
	//			.Size(FVector2D(LayoutSettings.Space, 0.0f))
	//		];

	//	// Add Loading widget on right
	//	HorizontalBox.Get().AddSlot()
	//		.VAlign(VAlign_Center)
	//		.HAlign(HAlign_Center)
	//		.AutoWidth()
	//		[
	//			LoadingWidget
	//		];
	//}


	//EVerticalAlignment VerticalAlignment;
	//// Set vertical alignment for widget
	//if (LayoutSettings.bIsWidgetAtBottom)
	//{
	//	VerticalAlignment = EVerticalAlignment::VAlign_Bottom;
	//}
	//else
	//{
	//	VerticalAlignment = EVerticalAlignment::VAlign_Top;
	//}

	//// Creating loading theme
	//Root->AddSlot()
	//.HAlign(LayoutSettings.BorderHorizontalAlignment)
	//.VAlign(VerticalAlignment)		
	//[
	//	SNew(SBorder)
	//	.HAlign(HAlign_Fill)
	//	.VAlign(VAlign_Fill)
	//	.BorderImage(&LayoutSettings.BorderBackground)
	//	.BorderBackgroundColor(FLinearColor::White)
	//	[
	//		SNew(SSafeZone)
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		.IsTitleSafe(true)
	//		.Padding(LayoutSettings.BorderPadding)
	//		[
	//			SNew(SDPIScaler)
	//			.DPIScale(this, &SClassicLayout::GetDPIScale)
	//			[					
	//				HorizontalBox
	//			]
	//		]
	//	]
	//];

	//// Construct loading complete text if enable
	//if (Settings.bShowLoadingCompleteText)
	//{
	//	Root->AddSlot()
	//		.VAlign(Settings.LoadingCompleteTextSettings.Alignment.VerticalAlignment)
	//		.HAlign(Settings.LoadingCompleteTextSettings.Alignment.HorizontalAlignment)
	//		.Padding(Settings.LoadingCompleteTextSettings.Padding)
	//		[
	//			SNew(SLoadingCompleteText, Settings.LoadingCompleteTextSettings)
	//		];
	//}

	//// Add root to this widget
	//ChildSlot
	//[
	//	Root
	//];
}