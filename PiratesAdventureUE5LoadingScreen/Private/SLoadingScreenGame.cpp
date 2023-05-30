#include "SLoadingScreenGame.h"
#include "STextWidget.h"
#include "LoadingScreenSettings.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Texture2D.h"
#include "SLoadingWidget.h"
#include "PiratesAdventureUE5LoadingScreen.h"
#include "LoadingScreenFunctionLibrary.h"

void SLoadingScreenGame::Construct(const FArguments& InArgs, const FPiratesAdventureLoadingScreenSettings& Settings)
{
	//To divide the Loading Screen into 3 horizontal bars and 2 vertical ones
	float TopBarSize = (1080.f - Settings.LoadText.LoadImageSize.Y) / 2.f;
	float SideBarSize = (1920.f - Settings.LoadText.LoadImageSize.X);
	int32 LoadTextIndex = ULoadingScreenFunctionLibrary::GetDisplayLoadIndex();
	FText LoadingText = FText::FromString("");
	FMargin ImagePadding = Settings.LoadText.ImagePadding;
	FVector2D ImageSize = Settings.LoadText.LoadImageSize;

	if(Settings.LoadText.LoadData.IsValidIndex(LoadTextIndex))
	{
		LoadingText = Settings.LoadText.LoadData[LoadTextIndex].LoadText;

		const FSoftObjectPath& LoadImageAsset = Settings.LoadText.LoadData[LoadTextIndex].LoadImage;
		UObject* LoadImageObject = LoadImageAsset.TryLoad();
		if (UTexture2D* LoadImage = Cast<UTexture2D>(LoadImageObject))
		{
			//We don't want a fixed size here. 
			LoadingImageBrush = FDeferredCleanupSlateBrush::CreateBrush(LoadImage, ImageSize);
		}
	}

	const FSoftObjectPath& HeaderImageAsset = Settings.Background.BackgroundImage;
	UObject* HeaderImageObject = HeaderImageAsset.TryLoad();
	if (UTexture2D* HeaderImage = Cast<UTexture2D>(HeaderImageObject))
	{
		//We don't want a fixed size here. 
		BGBrush = FDeferredCleanupSlateBrush::CreateBrush(HeaderImage);
	}
	
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		.Visibility(this, &SLoadingScreenGame::GetUIVisibility)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			//first create a super basic background color.
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.BorderBackgroundColor(Settings.Background.BackgroundColor)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			[
				SNew(SScaleBox)
				.Stretch(Settings.Background.BackgroundImageStretch)
				[
					//stick the header in.
					SNew(SImage)
					.Image(BGBrush.IsValid() ? BGBrush->GetSlateBrush() : nullptr)
				]
			]
		];

	//Let's set up the horizontal layer (and yes, these needs to be vertical boxes, due to the height)
	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);

	//First add the loading text
	Canvas.Get().AddSlot()
		.AutoSize(true)
		.Anchors(FAnchors(0.f, 0.5f, 0.f, 0.5f))
		.Alignment(FVector2D(0.f, 0.0f))
		.Offset(Settings.LoadText.TextPadding)
		.ZOrder(1)
		[
			////image is for testing the size of the area
			//SNew(SImage)
			//.ColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f))
			SNew(STextBlock)
			.ColorAndOpacity(Settings.LoadText.Appearance.ColorAndOpacity)
			.Font(Settings.LoadText.Appearance.Font)
			.ShadowOffset(Settings.LoadText.Appearance.ShadowOffset)
			.ShadowColorAndOpacity(Settings.LoadText.Appearance.ShadowColorAndOpacity)
			.Justification(Settings.LoadText.Appearance.Justification)
			.WrapTextAt(Settings.LoadText.TextWrapAt)
			.Text(LoadingText)
		];

	//Then let's add the text "the story so far"
	Canvas.Get().AddSlot()
		.AutoSize(true)
		.Anchors(FAnchors(0.f, 0.5f, 0.f, 0.5f))
		.Alignment(FVector2D(0.f, 0.0f))
		.Offset(Settings.LoadText.StorySoFarPadding)
		.ZOrder(1)
		[
			SNew(STextBlock)
			.ColorAndOpacity(Settings.LoadText.StorySoFarAppearance.ColorAndOpacity)
			.Font(Settings.LoadText.StorySoFarAppearance.Font)
			.ShadowOffset(Settings.LoadText.StorySoFarAppearance.ShadowOffset)
			.ShadowColorAndOpacity(Settings.LoadText.StorySoFarAppearance.ShadowColorAndOpacity)
			.Justification(Settings.LoadText.StorySoFarAppearance.Justification)
			.Text(Settings.LoadText.StorySoFarText)
		];

	//Then let's add the loading graphic
	Canvas.Get().AddSlot()
		.AutoSize(true)
		.Anchors(FAnchors(1.f, 0.0f, 1.f, 0.0f))
		.Alignment(FVector2D(0.f, 1.f)) //set the pivot to bottom right
		.Offset(FMargin(ImagePadding.Left - ImageSize.X, ImagePadding.Top + ImageSize.Y, ImagePadding.Right, ImagePadding.Bottom))
		.ZOrder(0)
		[
			SNew(SImage)
			.Image(LoadingImageBrush.IsValid() ? LoadingImageBrush->GetSlateBrush() : nullptr)
		];

	//Create a separate bar for the loading widget, since we really want to keep that the same as the start screen
	TSharedRef<SVerticalBox> HorizontalBox = SNew(SVerticalBox);
	TSharedRef<SLoadingWidget> LoadingWidget = SNew(SLoadingWidget, Settings.LoadingWidget);

	HorizontalBox.Get().AddSlot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.AutoHeight()
		[
			LoadingWidget
		];

	//HorizontalBars.Get().AddSlot()
	//	.HAlign(HAlign_Fill)
	//	.VAlign(VAlign_Fill)
	//	.AutoHeight()
	//	[
	//		SNew(SBox)
	//		.HeightOverride(TopBarSize)
	//		.HAlign(HAlign_Fill)
	//		[
	//			//Let's add text here later for the whole "the story so far"
	//			SNew(SImage)
	//			.ColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 0.6f))
	//		]
	//	];

	////this adds the middle bar, which contains the story so far text
	//HorizontalBars.Get().AddSlot()
	//	.HAlign(HAlign_Fill)
	//	.VAlign(VAlign_Fill)
	//	.AutoHeight()
	//	[
	//		SNew(SBox)
	//		.HeightOverride(Settings.LoadText.LoadImageSize.Y)
	//		[
	//			SNew(SHorizontalBox)
	//			+ SHorizontalBox::Slot()
	//			.HAlign(HAlign_Left)
	//			.VAlign(VAlign_Fill)
	//			.Padding(Settings.LoadText.TextPadding)
	//			[
	//				SNew(SBox)
	//				.WidthOverride(SideBarSize)
	//				[
	//					//image is for testing the size of the area
	//					//SNew(SImage)
	//					//.ColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f))
	//					SNew(STextBlock)
	//					.ColorAndOpacity(Settings.LoadText.Appearance.ColorAndOpacity)
	//					.Font(Settings.LoadText.Appearance.Font)
	//					.ShadowOffset(Settings.LoadText.Appearance.ShadowOffset)
	//					.ShadowColorAndOpacity(Settings.LoadText.Appearance.ShadowColorAndOpacity)
	//					.Justification(Settings.LoadText.Appearance.Justification)
	//					.WrapTextAt(Settings.LoadText.TextWrapAt)
	//					.Text(LoadingText)
	//				]
	//			]
	//		]
	//	];

	//HorizontalBars.Get().AddSlot()
	//	.HAlign(HAlign_Fill)
	//	.VAlign(VAlign_Fill)
	//	.AutoHeight()
	//	[
	//		SNew(SBox)
	//		.HeightOverride(TopBarSize)
	//		[
	//			//LoadingWidget
	//			SNew(SImage)
	//			.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 1.0f, 1.0f))
	//		]
	//	];

	////Then setup the vertical bars, needed for the story image
	//TSharedRef<SOverlay> VerticalBars = SNew(SOverlay);
	//
	//VerticalBars.Get().AddSlot()
	//	.HAlign(HAlign_Right)
	//	.VAlign(VAlign_Top)
	//	[
	//		SNew(SBox)
	//		.HeightOverride(1024.f)
	//		.WidthOverride(1024.f)
	//		[
	//			SNew(SImage)
	//			.ColorAndOpacity(FLinearColor(1.0f, 0.0f, 1.0f, 1.0f))
	//		]
	//	];

	//Add everything to Root, while making sure it has been scaled properly
	Root->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SSafeZone)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.IsTitleSafe(true)
		.Visibility(this, &SLoadingScreenGame::GetUIVisibility)
		[
			SNew(SDPIScaler)
			.DPIScale(this, &SLoadingScreenGame::GetDPIScale)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					Canvas
				]

				+ SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Right)
				[
					HorizontalBox
				]
				//Add the vertical bars later too
			]
		]
	];

	//finally add everything to the child slot
	ChildSlot
		[
			Root
		];

	/*
	//SNew(SHorizontalBox)
				//+SHorizontalBox::Slot()
				//.HAlign(HAlign_Fill)
				//.VAlign(VAlign_Fill)
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(BorderSize)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f))
					]
				]

				//Middle slot
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(512.f)
					[
						//If we want to have a section with a fixed with and height, we need to nest vertical and horizontal widgets, and use seperate boxes to box out both width and height
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride(1024.f)
							[
								SNew(SImage)
								.ColorAndOpacity(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f))
							]
						]

						//+ SHorizontalBox::Slot()
						//.HAlign(HAlign_Left)
						//.VAlign(VAlign_Fill)
						//[
						//	SNew(SImage)
						//	.ColorAndOpacity(FLinearColor(1.0f, 0.0f, 1.0f, 1.0f))
						//]
					]
				]

				+ SVerticalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(BorderSize)
					[
						SNew(SImage)
						.ColorAndOpacity(FLinearColor(0.0f, 0.0f, 1.0f, 1.0f))
					]
				]
	
	
	
	*/

	////Try to get start screen image. 
	//const FSoftObjectPath& HeaderImageAsset = Settings.Background.BackgroundImage;
	//UObject* HeaderImageObject = HeaderImageAsset.TryLoad();
	//if (UTexture2D* HeaderImage = Cast<UTexture2D>(HeaderImageObject))
	//{
	//	//We don't want a fixed size here. Just stretch it all over the damn place.	
	//	BGBrush = FDeferredCleanupSlateBrush::CreateBrush(HeaderImage);

	//	//since this is the start screen, we will assume by default that we need no stretching or tiling. Just the image put on once. 
	//	Root.Get().AddSlot()
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		[
	//			SNew(SScaleBox)
	//			.Stretch(Settings.Background.BackgroundImageStretch)
	//			[
	//				//stick the header in.
	//				SNew(SImage)
	//				.Image(BGBrush.IsValid() ? BGBrush->GetSlateBrush() : nullptr)
	//			]	
	//		];
	//}

	////make a horizontal box for the loader graphic, make the loader as well
	//TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);
	//TSharedRef<SLoadingWidget> LoadingWidget = SNew(SLoadingWidget, Settings.LoadingWidget);
	//TSharedRef<STextWidget> LoadingText = SNew(STextWidget, Settings.LoadText);

	////First add the loading text
	//VerticalBox.Get().AddSlot()
	//	.VAlign(VAlign_Top)
	//	.HAlign(HAlign_Fill)
	//	.AutoHeight()
	//	[
	//		//Perhaps have a chapter indicator in this box? Maybe a "The story so far..." or something?
	//		SNew(SSpacer)
	//		.Size(FVector2D(0.f, 0.f))
	//	];

	////Then add the loading text, but fix the size
	//VerticalBox.Get().AddSlot()
	//	.VAlign(VAlign_Center)
	//	.HAlign(HAlign_Fill)
	//	.MaxHeight(512.f)
	//	[
	//		LoadingText
	//	];

	////then the loader graphic
	//VerticalBox.Get().AddSlot()
	//	.VAlign(VAlign_Bottom)
	//	.HAlign(HAlign_Right)
	//	.AutoHeight()
	//	[
	//		LoadingWidget
	//	];

	////Finally add the whole thing to root to finish off the start screen.
	//Root->AddSlot()
	//	.HAlign(HAlign_Fill)
	//	.VAlign(VAlign_Bottom)
	//	[
	//		SNew(SSafeZone)
	//		.HAlign(HAlign_Fill)
	//		.VAlign(VAlign_Fill)
	//		.IsTitleSafe(true)
	//		[
	//			SNew(SDPIScaler)
	//			.DPIScale(this, &SLoadingScreenGame::GetDPIScale)
	//			[
	//				VerticalBox
	//			]
	//		]
	//	];

	//ChildSlot
	//	[
	//		Root
	//	];
}	

int32 SLoadingScreenGame::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//a simple tick delay for visualization, so our UI can at the very least get the DPI right before then
	if(!bIsVisible)
	{
		TickCount++;
		if(TickCount > 1)
			bIsVisible = true; 
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}