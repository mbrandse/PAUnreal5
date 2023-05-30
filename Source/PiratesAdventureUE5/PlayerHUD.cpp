// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "PlayerCharacter.h"
#include "PlayerHUDRingMenu.h"
#include "PlayerHUDContextPopup.h"
#include "PlayerHUDCharge.h"
#include "PlayerHUDHealth.h"
#include "Engine/InputDelegateBinding.h"
//this is for testing
#include "SLoadingScreenStart.h"
#include "Widgets/SWeakWidget.h"
#include "LoadingScreenSettings.h"
#include "Widgets/SCompoundWidget.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	HUDOwner = Cast<APlayerCharacter>(PlayerOwner->GetPawn());
	APlayerController* PC = Cast<APlayerController>(HUDOwner->GetController());

	if (ElementRingTemplate)
	{
		ElementRingInstance = CreateWidget<UPlayerHUDRingMenu>(PC, ElementRingTemplate);
		if (ElementRingInstance)
		{
			ElementRingInstance->ElementOwner = this;
			ElementRingInstance->AddToViewport();
			ElementRingInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (ToolRingTemplate)
	{
		ToolRingInstance = CreateWidget<UPlayerHUDRingMenu>(PC, ToolRingTemplate);
		if (ToolRingInstance)
		{
			ToolRingInstance->ElementOwner = this;
			ToolRingInstance->AddToViewport();
			ToolRingInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (ContextPopupTemplate)
	{
		ContextPopupInstance = CreateWidget<UPlayerHUDContextPopup>(PC, ContextPopupTemplate);
		if (ContextPopupInstance)
		{
			ContextPopupInstance->ElementOwner = this;
			ContextPopupInstance->AddToViewport();
			ContextPopupInstance->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

//void APlayerHUD::DrawHUD()
//{
//	Super::DrawHUD();
//
//	if (!MenuWidget.IsValid())
//	{
//		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
//		FLinearColor Color = Settings->StartupLoadingScreen.Background.BackgroundColor;
//		FGeometry Geometry = MenuWidget->GetPaintSpaceGeometry();
//		//MenuWidget = SNew(SLoadingScreenStart, Settings->StartupLoadingScreen);
//		//TSharedPtr<SCompoundWidget> NewWidget = SNew(SCompoundWidget);
//
//		//if (MenuWidget.IsValid())
//		//{
//		//	GEngine->GameViewport->AddViewportWidgetContent(
//		//		SNew(SWeakWidget)
//		//		.PossiblyNullContent(MenuWidget.ToSharedRef())
//		//	);
//		//}
//	}
//}

void APlayerHUD::EnableHUDInput(class APlayerController* PlayerController, bool bResetBindings)
{
	if (PlayerController)
	{
		// If it doesn't exist create it and bind delegates. We should also rebind everything in case we load from a save for instance
		if (!InputComponent
			|| bResetBindings)
		{	
			if (!InputComponent)
			{
				InputComponent = NewObject<UInputComponent>(this);
				InputComponent->RegisterComponent();
				InputComponent->bBlockInput = true; //to make sure we can completely override other inputcomponents for now.
				InputComponent->Priority = InputPriority;
			}

			if (bResetBindings)
			{
				InputComponent->ClearActionBindings();
				InputComponent->AxisBindings.Empty();
			}

			//Let's bind all HUD related options here
			InputComponent->BindAction("FaceRight", IE_Pressed, this, &APlayerHUD::InterfaceAcceptPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("FaceRight", IE_Released, this, &APlayerHUD::InterfaceAcceptRelease).bExecuteWhenPaused = true;
			InputComponent->BindAction("FaceDown", IE_Pressed, this, &APlayerHUD::InterfaceCancelPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("FaceDown", IE_Released, this, &APlayerHUD::InterfaceCancelRelease).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIUp", IE_Pressed, this, &APlayerHUD::InterfaceUpPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIUp", IE_Released, this, &APlayerHUD::InterfaceUpRelease).bExecuteWhenPaused = true;
			InputComponent->BindAction("UILeft", IE_Pressed, this, &APlayerHUD::InterfaceLeftPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("UILeft", IE_Released, this, &APlayerHUD::InterfaceLeftRelease).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIDown", IE_Pressed, this, &APlayerHUD::InterfaceDownPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIDown", IE_Released, this, &APlayerHUD::InterfaceDownRelease).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIRight", IE_Pressed, this, &APlayerHUD::InterfaceRightPress).bExecuteWhenPaused = true;
			InputComponent->BindAction("UIRight", IE_Released, this, &APlayerHUD::InterfaceRightRelease).bExecuteWhenPaused = true;

			if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
			{
				UInputDelegateBinding::BindInputDelegates(GetClass(), InputComponent);
			}
		}
		else
		{
			// Make sure we only have one instance of the InputComponent on the stack
			PlayerController->PopInputComponent(InputComponent);
		}

		PlayerController->PushInputComponent(InputComponent);
	}
}

//--------------------------------------------------------------------------------------------------------
// HUD INPUT FUNCTIONS
//--------------------------------------------------------------------------------------------------------

void APlayerHUD::InterfaceAcceptPress()
{
	ReceiveInterfaceAcceptPress();
}

void APlayerHUD::InterfaceAcceptRelease()
{
	ReceiveInterfaceAcceptRelease();
}

void APlayerHUD::InterfaceCancelPress()
{
	ReceiveInterfaceCancelPress();
}

void APlayerHUD::InterfaceCancelRelease()
{
	ReceiveInterfaceCancelRelease();
}

void APlayerHUD::InterfaceUpPress()
{
	ReceiveInterfaceUpPress();
}

void APlayerHUD::InterfaceUpRelease()
{
	ReceiveInterfaceUpRelease();
}

void APlayerHUD::InterfaceLeftPress()
{
	ReceiveInterfaceLeftPress();
}

void APlayerHUD::InterfaceLeftRelease()
{
	ReceiveInterfaceLeftRelease();
}

void APlayerHUD::InterfaceDownPress()
{
	ReceiveInterfaceDownPress();
}

void APlayerHUD::InterfaceDownRelease()
{
	ReceiveInterfaceDownRelease();
}

void APlayerHUD::InterfaceRightPress()
{
	ReceiveInterfaceRightPress();
}

void APlayerHUD::InterfaceRightRelease()
{
	ReceiveInterfaceRightRelease();
}