#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MoviePlayer.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Styling/SlateBrush.h"
#include "Framework/Text/TextLayout.h"
#include "LoadingScreenSettings.generated.h"

// Text appearance settings
USTRUCT(BlueprintType)
struct FTextAppearance
{
	GENERATED_BODY()

	/** Text color and opacity */
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateColor ColorAndOpacity = FSlateColor(FLinearColor::White);

	// The font to render the text with.
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FSlateFontInfo Font;

	/** Drop shadow offset in pixels */
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FVector2D ShadowOffset;

	/** Shadow color and opacity */
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	FLinearColor ShadowColorAndOpacity;

	/** How the text should be aligned with the margin. */
	UPROPERTY(BlueprintReadWrite, Config, EditAnywhere, Category = "Text Appearance")
	TEnumAsByte <ETextJustify::Type> Justification;
};

USTRUCT(BlueprintType)
struct FImageSequenceSettings
{
	GENERATED_BODY()

	/** An array of images for animating the loading icon.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (AllowedClasses = "Texture2D"))
	TArray<UTexture2D*> Images;

	/** Scale of the images.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/**
	 * Time in second to update the images, the smaller value the faster of the animation. A zero value will update the images every frame.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (UIMax = 1.00, UIMin = 0.00, ClampMin = "0", ClampMax = "1"))
	float Interval = 0.05f;

	/**
	* Time in second for the final frame. Make longer than Interval in case we need a delay
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (UIMax = 1.00, UIMin = 0.00, ClampMin = "0", ClampMax = "1"))
		float FinalInterval = 0.05f;
};

/**
 * Background widget for the widget loading screen
 */
USTRUCT(BlueprintType)
struct PIRATESADVENTUREUE5LOADINGSCREEN_API FBackgroundSettings
{
	GENERATED_BODY()

	// The background image for the loading screen. Should be set to tiling in slate.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background", meta = (AllowedClasses = "Texture2D"))
		FSoftObjectPath BackgroundImage;

	// The background color in case we don't want to use an image here. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
		FLinearColor BackgroundColor = FLinearColor::Black;

	// The size of the background image. Necessary in case we ignore the image stretch type. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
		FVector2D BackgroundSize = FVector2D(1024.f, 128.f);

	// The scaling type to apply to images. //DOUBLECHECK IF THERE IS A TILING OPTION TOO!
	// This is for a scaling box. Is there a tiling box too?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Background")
		TEnumAsByte<EStretch::Type> BackgroundImageStretch = EStretch::ScaleToFit;


};

/**
 * Loading widget settings
 */
USTRUCT(BlueprintType)
struct PIRATESADVENTUREUE5LOADINGSCREEN_API FLoadingWidgetSettings
{
	GENERATED_BODY()

	FLoadingWidgetSettings();

	/** Render transform translation of the loading icon.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
		FVector2D TransformTranslation = FVector2D(0.0f, 0.0f);

	/** Render transform scale of the loading icon, a negative value will flip the icon.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
		FVector2D TransformScale = FVector2D(1.0f, 1.0f);

	/** Render transform pivot of the loading icon (in normalized local space).*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
		FVector2D TransformPivot = FVector2D(0.5f, 0.5f);

	// Text displayed under the animated icon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
		bool bShowLoadingText = false;

	// Text displayed under the animated icon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (EditCondition = "bShowLoadingText"))
		FText LoadingText;

	// Loading text appearance settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting", meta = (EditCondition = "bShowLoadingText"))
		FTextAppearance Appearance;

	/** Image Sequence settings. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Widget Setting")
		TArray<FImageSequenceSettings> ImageSequenceSettings;

	/** Empty space between the loading widget and screen bottom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
		float LoadingWidgetBottomMargin = 1.0f;

	/** Empty space between the loading text and the loading widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
		float LoadingWidgetSideMargin = 1.0f;

	/** Empty space between the loading text and screen bottom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
		float LoadingTextBottomMargin = 1.0f;

	/** Empty space between the loading text and screen right */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
		float LoadingTextSideMargin = 1.0f;

	//do we still need this?
	/** Hide the loading widget when the level loading is complete*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading Widget Setting")
		bool bHideLoadingWidgetWhenCompletes = false;
};

USTRUCT(BlueprintType)
struct PIRATESADVENTUREUE5LOADINGSCREEN_API FLoadStoryData
{
	GENERATED_BODY()

	// The text to show in the load screen. In this game, we are going to turn this into a story based text. Do we need the multiline here?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Story Data", meta = (MultiLine = true))
		FText LoadText;

	// The image to accompany the text.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Story Data")
		FSoftObjectPath LoadImage;
};

/**
 * Loading flavour text settings
 */
USTRUCT(BlueprintType)
struct PIRATESADVENTUREUE5LOADINGSCREEN_API FLoadTextSettings
{
	GENERATED_BODY()

	FLoadTextSettings();

	// The text to show in the load screen. In this game, we are going to turn this into a story based text.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Load Screen Text Settings", meta = (MultiLine = true))
		TArray<FLoadStoryData> LoadData;

	// Load Screen text appearance settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FTextAppearance Appearance;

	// The size of the text before it's wrapped to the next line
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		float TextWrapAt;

	// The size of the text before it's wrapped to the next line
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FMargin TextPadding;

	// The space between text and image. Image size largely determines layout as well.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FVector2D LoadImageSize;

	// The space between text and image. Image size largely determines layout as well.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FMargin ImagePadding;

	// Text displayed under the animated icon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FText StorySoFarText;

	// Loading text appearance settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FTextAppearance StorySoFarAppearance;

	// The size of the text before it's wrapped to the next line
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tip Settings")
		FMargin StorySoFarPadding;
};

/**
 * Loading Screen Settings
 */
USTRUCT(BlueprintType)
struct PIRATESADVENTUREUE5LOADINGSCREEN_API FPiratesAdventureLoadingScreenSettings
{
	GENERATED_BODY()	

	// The minimum time that a loading screen should be opened for, -1 if there is no minimum time. I recommend set it to -1.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	float MinimumLoadingScreenDisplayTime = -1;
	
	// If true, the loading screen will disappear as soon as loading is done.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAutoCompleteWhenLoadingCompletes = true;

	// If true, movies can be skipped by clicking the loading screen as long as loading is done.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bMoviesAreSkippable = true;

	/** If true loading screens here cannot have any uobjects of any kind or use any engine features at all. This will start the movies very early as a result on platforms that support it */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowInEarlyStartup = false;

	/** If true, this will call the engine tick while the game thread is stalled waiting for a loading movie to finish. This only works for post-startup load screens and is potentially unsafe */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	bool bAllowEngineTick = false;

	/**
	 * All movie files must be located at Content/Movies/ directory. Suggested format: MPEG-4 Movie (mp4). 
	 * Enter file path/name without the extension.
	 * E.g., if you have a movie name my_movie.mp4 in the 'Content/Movies' folder, then enter my_movie in the input field.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movies Settings")
	TArray<FString> MoviePaths;

	//do we still need this? Our implementation is not variable.
	/** 
	 * Should we show the loading screen widgets (background/tips/loading widget)? Generally you'll want to set this to false if you just want to show a movie.
	 */ 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	bool bShowWidgetOverlay = true;		

	/** Background widget for the loading screen. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FBackgroundSettings Background;	
	
	/** Tip widget for the loading screen. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FLoadTextSettings LoadText;

	/** Loading widget for the loading screen. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Loading Screen Settings")
	FLoadingWidgetSettings LoadingWidget;
};

/**
 * Loading Screen Settings 
 */
UCLASS(Config = "Game", defaultconfig, meta = (DisplayName = "Pirates Adventure Loading Screen"))
class PIRATESADVENTUREUE5LOADINGSCREEN_API ULoadingScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	ULoadingScreenSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * The startup loading screen when you first open the game. Setup any studio logo movies here.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	FPiratesAdventureLoadingScreenSettings StartupLoadingScreen;

	/**
	 * The default loading screen that shows up whenever you open a new level.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	FPiratesAdventureLoadingScreenSettings DefaultLoadingScreen;
};
