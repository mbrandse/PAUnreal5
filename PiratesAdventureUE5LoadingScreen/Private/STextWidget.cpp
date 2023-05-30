#include "STextWidget.h"
#include "LoadingScreenSettings.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"
#include "LoadingScreenFunctionLibrary.h"
#include "Slate/DeferredCleanupSlateBrush.h"

//TODO: MAKE ANOTHER ONE LIKE THIS, BUT FOR IMAGE. WE CAN JUST USE THE SAME VARIABLE IN THE ACTUAL LOADSCREEN CONSTRUCTION

void STextWidget::Construct(const FArguments& InArgs, const FLoadTextSettings& Settings)
{
	if (Settings.LoadData.Num() > 0
		&& Settings.LoadData.IsValidIndex(ULoadingScreenFunctionLibrary::GetDisplayLoadIndex()))
	{
		int32 TextIndex = ULoadingScreenFunctionLibrary::GetDisplayLoadIndex();

		//Try to get start screen image. 
		const FSoftObjectPath& LoadImageAsset = Settings.LoadData[TextIndex].LoadImage;
		UObject* LoadImageObject = LoadImageAsset.TryLoad();
		if (UTexture2D* LoadImage = Cast<UTexture2D>(LoadImageObject))
		{
			LoadImageBrush = FDeferredCleanupSlateBrush::CreateBrush(LoadImage, Settings.LoadImageSize);
		}

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
				.Font(Settings.Appearance.Font)
				.ShadowOffset(Settings.Appearance.ShadowOffset)
				.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
				.Justification(Settings.Appearance.Justification)
				.WrapTextAt(Settings.TextWrapAt)
				.Text(Settings.LoadData[TextIndex].LoadText)
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SSpacer)
				//.Size(FVector2D(Settings.TextAndImageSpacing, 0.f))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				[				
					SNew(SImage)
					.Image(LoadImageBrush.IsValid() ? LoadImageBrush->GetSlateBrush() : nullptr)
				]
			]
		];
		
	}
}
