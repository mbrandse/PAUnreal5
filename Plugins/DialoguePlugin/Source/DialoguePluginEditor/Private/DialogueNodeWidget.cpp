#include "DialogueNodeWidget.h"
#include "DialoguePluginEditorPrivatePCH.h"
#include "DialogueEditor.h"
#include "DialogueViewportWidget.h"
#include "DialogueEditorStyle.h"
#include "Dialogue.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/MenuStack.h"
#include "Editor/UnrealEd/Public/ScopedTransaction.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "SDialogueViewportWidget"
 
void SDialogueNodeWidget::Construct(const FArguments& InArgs)
{	
	Id = InArgs._Id;
	NodeIndex = InArgs._NodeIndex;
	Dialogue = InArgs._Dialogue; // for styles, TD: move somewhere?
	Owner = InArgs._Owner;
	SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &SDialogueNodeWidget::GetNodeVisibility)));
	
	ChildSlot
	[
		SNew(SOverlay)
			/*
			* NodeImage
			*/
			+ SOverlay::Slot() // the node itself
			[
				SNew(SImage)
				.Image(this, &SDialogueNodeWidget::GetNodeStyle)
				.Visibility(EVisibility::Visible)
			]

			/*
			* a Spacer that gives a minimum size to the node
			*/
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SSpacer)
				.Size(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeMinSize)))
			]

			/*
			* horizontal box containing top left icons
			*/
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			.Padding(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetLeftCornerPadding))) //.Padding(5, 0, 0, 0) at Zoom 1:1
			[
				SNew(SHorizontalBox)
				.RenderTransform(TAttribute<TOptional<FSlateRenderTransform>>::Create(TAttribute<TOptional<FSlateRenderTransform>>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetIconScale)))
				.RenderTransformPivot(FVector2D(0, 0))
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 3, 0, 0)
				[
					SNew(SImage)
					.Image(FDialogueEditorStyle::Get()->GetBrush("EventIcon"))
					.ColorAndOpacity(FLinearColor(246.0f / 255.0f, 207.0f / 255.0f, 6.0f / 255.0f, 1.0f))
					.Visibility(this, &SDialogueNodeWidget::GetEventIconVisibility)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 3, 0, 0)
				[
					SNew(SImage)
					.Image(FDialogueEditorStyle::Get()->GetBrush("ConditionIcon"))
					.ColorAndOpacity(FLinearColor(106.0f / 255.0f, 221.0f / 255.0f, 214.0f / 255.0f, 1.0f)) // FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
					.Visibility(this, &SDialogueNodeWidget::GetConditionIconVisibility)
				]
				//CUSTOM ADDITION; NODE TITLE
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10, 3, 0, 0)
				[
					SAssignNew(NodeTextTitleBlock, STextBlock)
					.Justification(ETextJustify::Left)
					.Text(this, &SDialogueNodeWidget::GetNodeTitleText)
					.Visibility(this, &SDialogueNodeWidget::GetTitleVisibility)
					.WrapTextAt(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeTextWrapLength)))
					.Font(TAttribute<FSlateFontInfo>::Create(TAttribute<FSlateFontInfo>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeFont)))
					.ColorAndOpacity(FLinearColor(255.0f / 255.0f, 200.0f / 255.0f, 0.f, 1.0f))
				]
				//END CUSTOM ADD
			]

			/*
			* horizontal box containing top right icons
			*/
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			.Padding(0, 0, 0, 0) // left top right bottom
			[
				SNew(SHorizontalBox)
				.RenderTransform(TAttribute<TOptional<FSlateRenderTransform>>::Create(TAttribute<TOptional<FSlateRenderTransform>>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetIconScale)))
				.RenderTransformPivot(FVector2D(1, 0))
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.FillWidth(1)
				.Padding(0, 3, 10, 0)
				[
					SNew(SImage)
					.Image(FDialogueEditorStyle::Get()->GetBrush("VoiceIcon"))
					.ColorAndOpacity(FLinearColor(106.0f / 255.0f, 174.0f / 255.0f, 101.0f / 255.0f, 1.0f))
					.Visibility(this, &SDialogueNodeWidget::GetSoundIconVisibility)
				]
			]

			/*
			* text block - displayed only when out of Editing mode
			*/
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(TAttribute<FMargin>(this, &SDialogueNodeWidget::GetTextMargin)) // left top right bottom
			[
				SAssignNew(NodeTextBlock, STextBlock)
				.Justification(ETextJustify::Left)
				.Text(this, &SDialogueNodeWidget::GetNodeText)
				.Visibility(this, &SDialogueNodeWidget::GetTextBlockVisibility) // @TODO: make a getter?
				.WrapTextAt(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeTextWrapLength))) // 250 by default at 1:1 zoom level
				.Font(TAttribute<FSlateFontInfo>::Create(TAttribute<FSlateFontInfo>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeFont)))
			]

			/*
			* editable text - displayed only when in Editing mode
			*/
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(TAttribute<FMargin>(this, &SDialogueNodeWidget::GetTextMargin))
			[
				SAssignNew(NodeField, SMultiLineEditableText)
				.Text(this, &SDialogueNodeWidget::GetFieldText)
				.Visibility(this, &SDialogueNodeWidget::GetTextFieldVisibility)
				.WrapTextAt(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeTextWrapLength))) // 250 by default at 1:1 zoom level
				.ModiferKeyForNewLine(EModifierKey::Shift)
				.OnTextCommitted(this, &SDialogueNodeWidget::TextCommited, Id)
				.SelectAllTextWhenFocused(true)
				.Font(TAttribute<FSlateFontInfo>::Create(TAttribute<FSlateFontInfo>::FGetter::CreateSP(Owner, &SDialogueViewportWidget::GetNodeFont)))
			]
	];
}

/*
* Performs culling. If the node isn't on screen, then it's collapsed. Boosts performance drammatically for dialogues with a large number of nodes.
*/
EVisibility SDialogueNodeWidget::GetNodeVisibility() const
{
	return isVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FMargin SDialogueNodeWidget::GetTextMargin() const
{
	if (!isVisible)
	{
		return FMargin(0, 0, 0, 0);
	}

	float marg10 = 10 * Owner->GetZoomAmount();
	float marg17 = 16 * Owner->GetZoomAmount();

	if (Owner->GetZoomLevel() <= 4)
	{
		return FMargin(0, 0, 0, 0);
	}	

	if (Owner->GetZoomLevel() > 6 && (Dialogue->Data[NodeIndex].Events.Num() > 0 || Dialogue->Data[NodeIndex].Conditions.Num() > 0 || Dialogue->Data[NodeIndex].Sound || Dialogue->Data[NodeIndex].DialogueWave))
	{
		return FMargin(marg10, marg17, marg10, marg10);
	}
	else
	{
		return FMargin(marg10, marg10, marg10, marg10);
	}
}

EVisibility SDialogueNodeWidget::GetTextFieldVisibility() const
{
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Collapsed;
	}

	return (Owner->EditTextNode == Id) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SDialogueNodeWidget::GetEventIconVisibility() const
{
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Collapsed;
	}

	return Dialogue->Data[NodeIndex].Events.Num() > 0 ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

EVisibility SDialogueNodeWidget::GetConditionIconVisibility() const
{
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Collapsed;
	}

	return Dialogue->Data[NodeIndex].Conditions.Num() > 0 ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

EVisibility SDialogueNodeWidget::GetSoundIconVisibility() const
{
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Collapsed;
	}

	return (Dialogue->Data[NodeIndex].Sound || Dialogue->Data[NodeIndex].DialogueWave) ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

EVisibility SDialogueNodeWidget::GetTextBlockVisibility() const
{
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 4)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Hidden;
	}

	return (Owner->EditTextNode == Id) ? EVisibility::Collapsed : EVisibility::Visible;
}

void SDialogueNodeWidget::TextCommited(const FText& NewText, ETextCommit::Type CommitInfo, int32 id)
{
	if (NewText.EqualTo(Dialogue->Data[NodeIndex].Text)) return;
	const FScopedTransaction Transaction(LOCTEXT("TextCommited", "Edited Node Text"));
	Dialogue->Modify();
	TOptional<FString> keyOpt = FTextInspector::GetKey(Dialogue->Data[NodeIndex].Text);
	TOptional<FString> nsOpt = FTextInspector::GetNamespace(Dialogue->Data[NodeIndex].Text);	
	Dialogue->Data[NodeIndex].Text = FText::ChangeKey(
		FTextKey(nsOpt.IsSet() ? nsOpt.GetValue() : ""),
		FTextKey(keyOpt.IsSet() ? keyOpt.GetValue() : ""),
		NewText
	);	
	Owner->EditTextNode = -1;
	Owner->DialogueEditorPtr.Pin()->refreshDetails = true;
}

int32 SDialogueNodeWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void SDialogueNodeWidget::Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	NodeSize = AllottedGeometry.GetLocalSize(); //TD: is it possible to bind it?
}

FReply SDialogueNodeWidget::OnMouseButtonDoubleClick(const FGeometry & InMyGeometry, const FPointerEvent & InMouseEvent)
{
	bJustDoubleClicked = true;
	return FReply::Unhandled();
}

void SDialogueNodeWidget::OnMouseEnter(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
{
	// if mouse is over a node widget after we stopped linkingAndPanning, try to link the node
	if (Dialogue->isLinking && !Owner->isLinkingAndCapturing) 
	{
		// we shouldn't link a node to itself
		if (Id != Dialogue->Data[Dialogue->LinkingFromIndex].id) 
		{
			// and we shouldn't link a node that's already linked
			if (!Dialogue->Data[Dialogue->LinkingFromIndex].Links.Contains(Id))
			{
				//PA MODIFICATION
				//finally, add some custom conditions for special nodes. 
				//1. We cannot link a node to an event node. 
				//2. We cannot link an answer to a player or other answer.

				bool bFaultyChoiceLink = false;
				FDialogueNode SampleNode = Dialogue->GetNodeById(Id);

				//make sure we cannot link a choice to another choice or player
				if (Dialogue->Data[Dialogue->LinkingFromIndex].NodeOwner == EDialogueNodeOwner::DNO_CHOICE
					&& (SampleNode.NodeOwner == EDialogueNodeOwner::DNO_CHOICE
						|| SampleNode.NodeOwner == EDialogueNodeOwner::DNO_PLAYER))
				{
					bFaultyChoiceLink = true;
				}

				//and make sure we cannot link from an NPC to a choice either
				if (Dialogue->Data[Dialogue->LinkingFromIndex].NodeOwner == EDialogueNodeOwner::DNO_NPC
					&& SampleNode.NodeOwner == EDialogueNodeOwner::DNO_CHOICE)
				{
					bFaultyChoiceLink = true;
				}

				//Finally, make sure we cannot link anything back to an event
				if (!bFaultyChoiceLink
					&& SampleNode.NodeOwner != EDialogueNodeOwner::DNO_EVENT)
				{
					const FScopedTransaction Transaction(LOCTEXT("AddLink", "Add Link"));
					Dialogue->Modify();

					Dialogue->Data[Dialogue->LinkingFromIndex].Links.Add(Id);
					SortParentsLinks();
				}
				//END PA MODIFICATION
			}
		}
		// whatever node we entered, we stop linking
		Dialogue->isLinking = false; 
	}
}

const FSlateBrush* SDialogueNodeWidget::GetNodeStyle() const
{	
	if (Id == 0) return FDialogueEditorStyle::Get()->GetBrush("StartNodeStyle");

	FDialogueNode CurrentNode;

	switch (Dialogue->Data[NodeIndex].NodeOwner)
	{
	case EDialogueNodeOwner::DNO_NPC:
		return FDialogueEditorStyle::Get()->GetBrush("NpcNodeStyle");

	case EDialogueNodeOwner::DNO_PLAYER:
		return FDialogueEditorStyle::Get()->GetBrush("PlayerNodeStyle");

	case EDialogueNodeOwner::DNO_CHOICE:
		for (int32 Idx = 0; Idx < Dialogue->Data.Num(); Idx++)
		{
			if (Dialogue->Data[Idx].id == Id)
			{
				CurrentNode = Dialogue->Data[Idx];
				CurrentNode.NodeType = EDialogueNodeType::DNT_NORMAL;
				Dialogue->Data[Idx] = CurrentNode;
				break;
			}
		}
		return FDialogueEditorStyle::Get()->GetBrush("ChoiceNodeStyle");

	case EDialogueNodeOwner::DNO_EVENT:
		return FDialogueEditorStyle::Get()->GetBrush("EventNodeStyle");

	default:
		return FDialogueEditorStyle::Get()->GetBrush("PlayerNodeStyle");
	}

	//return Dialogue->Data[NodeIndex].isPlayer ? FDialogueEditorStyle::Get()->GetBrush("PlayerNodeStyle") : FDialogueEditorStyle::Get()->GetBrush("NpcNodeStyle");
}

FText SDialogueNodeWidget::GetNodeText() const
{
	if (!isVisible)
	{
		return FText::GetEmpty();
	}

	if (Dialogue->Data[NodeIndex].id == 0)
	{
		return FText::FromString("Start");
	}

	FDialogueNode SampleNode = Dialogue->GetNodeById(Id);
	if (SampleNode.NodeOwner == EDialogueNodeOwner::DNO_EVENT)
	{
		FString BaseName = "Event: ";
		BaseName.Append(SampleNode.TextEventName.ToString());

		return FText::FromString(BaseName);
	}

	FText NodeText = Dialogue->Data[NodeIndex].Text;

	int maxTextLength = 300; //@TODO: change 300 to user's choice

	if (NodeText.ToString().Len() > maxTextLength)
	{
		FString Output;
		
		Output += NodeText.ToString();
		Output.RemoveAt(maxTextLength, NodeText.ToString().Len() - maxTextLength, true);
		Output += TEXT("...");

		return FText::FromString(Output);
	}
	
	return SearchTextForSpecialCommands(NodeText);
}

//----------------------------------------------------------------------------------------------------
// PA SPECIFIC FUNCTIONALITY START
//----------------------------------------------------------------------------------------------------

//TODO:Move this to the Dialogue Node. We may have to calculate this in realtime, if we allow for dynamic strings in text. 
//We may also have to combine this with the Temaran Widget if we get to that point, considering that will have many more preprocessor commands. 
FText SDialogueNodeWidget::SearchTextForSpecialCommands(FText InText) const
{
	FString ReturnText = InText.ToString();

	if (!InText.IsEmpty())
	{
		bool bReachedEndOfText = false;

		int32 CurrentIdx = 0; //our current index;

		TArray<FDialogueEvent> DialogueEvents;
		FDialogueEvent CurrentEvent;
		CurrentEvent.StartIdx = -1;
		CurrentEvent.EndIdx = -1;
		CurrentEvent.ContentIdx = -1;
		CurrentEvent.Command.Empty();
		CurrentEvent.Content.Empty();

		while (!bReachedEndOfText)
		{
			//HandAdd any \n directives. See if that works. 
			FString EnterDirective = ReturnText.Mid(CurrentIdx, 2);
			if (EnterDirective == "\n")
			{
				ReturnText.RemoveAt(CurrentIdx, 2);
				ReturnText.InsertAt(CurrentIdx, "\n");
			}

			if (CurrentEvent.Command.IsEmpty())
			{
				if (CurrentEvent.StartIdx == -1)
				{
					FString CurrentLetter = ReturnText.Mid(CurrentIdx, 1);
					if (CurrentLetter == "<")
					{
						CurrentEvent.StartIdx = CurrentIdx;
					}
				}
				else
				{
					FString CurrentLetter = ReturnText.Mid(CurrentIdx, 1);
					if (CurrentLetter == ":" || CurrentLetter == "=")
					{
						CurrentEvent.Command = ReturnText.Mid(CurrentEvent.StartIdx + 1, CurrentIdx - (CurrentEvent.StartIdx + 1)); //we don't add one, so we don't get weird stuff
						CurrentEvent.ContentIdx = CurrentIdx + 1;
					}
					else if (CurrentLetter == ">") //in case we have a faulty or different preprocessor command.
					{
						CurrentEvent.EndIdx = CurrentIdx;
						DialogueEvents.Add(CurrentEvent);

						//reset current event again so we can start over until we reach the end of the string.
						CurrentEvent.StartIdx = -1;
						CurrentEvent.EndIdx = -1;
						CurrentEvent.ContentIdx = -1;
						CurrentEvent.Command.Empty();
						CurrentEvent.Content.Empty();
					}
				}
			}
			else
			{
				FString CurrentLetter = ReturnText.Mid(CurrentIdx, 1);
				if (CurrentLetter == ">")
				{
					CurrentEvent.Content = ReturnText.Mid(CurrentEvent.ContentIdx, CurrentIdx - CurrentEvent.ContentIdx);
					CurrentEvent.EndIdx = CurrentIdx;
					DialogueEvents.Add(CurrentEvent);

					//reset current event again so we can start over until we reach the end of the string.
					CurrentEvent.StartIdx = -1;
					CurrentEvent.EndIdx = -1;
					CurrentEvent.ContentIdx = -1;
					CurrentEvent.Command.Empty();
					CurrentEvent.Content.Empty();
				}
			}

			CurrentIdx++;
			if (CurrentIdx > ReturnText.Len())
				bReachedEndOfText = true;
		}

		FDialogueNode CurrentNode = Dialogue->GetNodeById(Id);

		//Clean the current node arrays, otherwise we will end up with slack.
		CurrentNode.TextEventLinkers.Empty();
		CurrentNode.TextSpeedLinkers.Empty();

		if (DialogueEvents.Num() != 0)
		{
			FString TempString;
			int32 CommandIdxOffset = 0;

			//TODO: do this with a normal Index. This way you can accurately set the StartIdx, since the current version doesn't take into account removal of 
			//preprocessor directives.
			for (FDialogueEvent DEvent : DialogueEvents)
			{
				bool bEventIsLegitimate = false;

				if (DEvent.Command == "Event")
				{
					for (FDialogueNode DialogueNode : Dialogue->Data)
					{
						if (DialogueNode.TextEventName == FName(*DEvent.Content))
						{
							bEventIsLegitimate = true;
							break;
						}
					}

					if (bEventIsLegitimate)
					{
						CurrentNode.TextEventLinkers.Add(DEvent.StartIdx - CommandIdxOffset, FName(*DEvent.Content));
						CommandIdxOffset += DEvent.EndIdx - DEvent.StartIdx;
					}
					else
					{
						return FText::FromString("Error in a 'Event' command");
					}
				}
				else if (DEvent.Command == "Speed")
				{
					float Speed = 0.f;
					if (DEvent.Content == "Reset")
					{
						Speed = -1.f;
					}
					else
					{
						Speed = FCString::Atof(*DEvent.Content);
					}

					if (Speed != 0.f)
					{
						CurrentNode.TextSpeedLinkers.Add(DEvent.StartIdx - CommandIdxOffset, Speed);
						CommandIdxOffset += DEvent.EndIdx - DEvent.StartIdx;
					}
					else
					{
						return FText::FromString("Error in a 'Speed' command");
					}
				}
				else
				{
					//this basically means we have a faulty or unrecognized command. Since we still need to account for 
					//the place of the letters for other events, let's increment the CommandIdxOffset
					CommandIdxOffset += DEvent.EndIdx - DEvent.StartIdx;
				}
			}

			//finally clean up the return string, to take out all preprocessor commands
			for (int32 Idx = DialogueEvents.Num() - 1; Idx > -1; Idx--)
			{
				FDialogueEvent DEvent = DialogueEvents[Idx];
				ReturnText.RemoveAt(DEvent.StartIdx, DEvent.EndIdx - DEvent.StartIdx + 1);
			}
		}

		CurrentNode.TextProcessed = FText::FromString(ReturnText);

		//since we are just saving the data of the struct to a new one, we actually have to update the dialogue data to match. 
		//this needs to be done outside of the loop, or empty arrays are not saved correctly. 
		for (int32 Idx = 0; Idx < Dialogue->Data.Num(); Idx++)
		{
			if (Dialogue->Data[Idx].id == Id)
			{
				Dialogue->Data[Idx] = CurrentNode;
				break;
			}
		}
	}

	return FText::FromString(ReturnText);
}

FText SDialogueNodeWidget::GetNodeTitleText() const
{
	if (!isVisible)
	{
		return FText::FromString("");
	}

	FDialogueNode SampleNode = Dialogue->GetNodeById(Id);

	if (Id > 0
		&& SampleNode.NodeOwner != EDialogueNodeOwner::DNO_EVENT)
	{
		FFormatNamedArguments Args;
		Args.Add("NPCIndex", Dialogue->Data[NodeIndex].NPCIndex);

		switch (Dialogue->Data[NodeIndex].NodeOwner)
		{
		case EDialogueNodeOwner::DNO_PLAYER:
			return FText::FromString("Morgan");

		case EDialogueNodeOwner::DNO_CHOICE:
			return FText::FromString("Morgan Choice");

		case EDialogueNodeOwner::DNO_NPC:
			return FText::Format(NSLOCTEXT("PiratesAdventureEditor", "NodeTitle_NPC", "NPC {NPCIndex}"), Args);

		default:
			return FText::FromString("Morgan");
		}
	}

	return FText::FromString("");
}

EVisibility SDialogueNodeWidget::GetTitleVisibility() const
{
	//Maybe add something conditional for nodes that don't have title text.
	if (!isVisible)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 4)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetZoomLevel() <= 6)
	{
		return EVisibility::Hidden;
	}

	//return (Owner->EditTextNode == Id) ? EVisibility::Collapsed : EVisibility::Visible;
	return EVisibility::Visible;
}

//----------------------------------------------------------------------------------------------------
// PA SPECIFIC FUNCTIONALITY END
//----------------------------------------------------------------------------------------------------

FText SDialogueNodeWidget::GetFieldText() const
{
	if (!isVisible)
	{
		return FText::FromString("");
	}

	return Dialogue->Data[NodeIndex].Text;
}

FReply SDialogueNodeWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Handled();

	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		Reply.DetectDrag(SharedThis(this), EKeys::RightMouseButton);
	}
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Reply.DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return Reply;
}

FReply SDialogueNodeWidget::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) && !Dialogue->isLinking)
	{
		Owner->isPanning = true;
	}
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (Owner->bBreakingLinksMode)
			Owner->bBreakingLinksMode = false;

		const FScopedTransaction Transaction(LOCTEXT("DragNodes", "Drag Nodes"));
		Dialogue->Modify();

		// offset = local coords of the center of the node - local coords of the mouse click (inside the node)
		Owner->draggingOffset = FVector2D(MyGeometry.GetLocalSize().X / 2, MyGeometry.GetLocalSize().Y / 2) - MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		Owner->StartDraggingIndex(NodeIndex);
	}
	return FReply::Handled();
}

FReply SDialogueNodeWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	/* Left mouse button UP */
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{	
		if ((bJustDoubleClicked || (Owner->SelectedNodes.Num() == 1 && Owner->SelectedNodes.Contains(Id) && NodeTextBlock->IsHovered())) && Id != 0)
		{
			bJustDoubleClicked = false;

			// Don't enter Edit mode if we're at -6 zoom or lower.
			if (Owner->GetZoomLevel() > 6)
			{
				NodeField->SetText(GetFieldText());
				Owner->EditTextNode = Id;
				FSlateApplication::Get().SetKeyboardFocus(NodeField);
			}

			return FReply::Handled();
		}

		if (Owner->bBreakingLinksMode)
		{
			// Breaking links between Id and Owner->breakingLinksFromId
			BreakLinksWithNode();
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}
	/* Right mouse button UP */
	else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (Owner->bBreakingLinksMode)
		{
			Owner->bBreakingLinksMode = false;
		}

		if (!Owner->SelectedNodes.Contains(Id))
		{
			Owner->SelectNodes(Id);
			Dialogue->CurrentNodeId = Id;
			Owner->ForceRefresh();
		}

		if (Dialogue->isLinking) // if RMB UP on a node when linking, cancel linking
		{
			Dialogue->isLinking = false;
			return FReply::Handled();
		}

		FMenuBuilder MenuBuilder(true, NULL);

		FUIAction AddLinkAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnAddLink));
		FUIAction AddPCAnswerAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnAddPcAnswer));
		//FUIAction AddNPCAnswerAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnAddNpcAnswer));
		FUIAction BreakOutLinksAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnBreakOutLinks));
		FUIAction BreakInLinksAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnBreakInLinks));
		FUIAction ChangePcNpcAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::OnChangePcNpc));
		FUIAction DeleteAction(FExecuteAction::CreateSP(Owner, &SDialogueViewportWidget::DeleteOneNode, Id));
		FUIAction DeleteAllAction(FExecuteAction::CreateSP(Owner, &SDialogueViewportWidget::DeleteSelected));
		FUIAction BreakLinksModeAction(FExecuteAction::CreateSP(this, &SDialogueNodeWidget::BreakLinksMode));

		MenuBuilder.BeginSection(NAME_None, NSLOCTEXT("PropertyView", "ExpansionHeading", "Dialogue"));
		
		// can't give a PC answer to a previous PC node
		if (Dialogue->Data[NodeIndex].NodeOwner == EDialogueNodeOwner::DNO_PLAYER)
		{
			MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "AddPcAnswer", "Add PC Answer"), NSLOCTEXT("PropertyView", "AddPcAnswer_ToolTip", "Adds a child node (PC Answer)"), FSlateIcon(), AddPCAnswerAction);
		}

		//MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "AddNpcAnswer", "Add NPC Answer"), NSLOCTEXT("PropertyView", "AddNpcAnswer_ToolTip", "Adds a child node (NPC Answer)"), FSlateIcon(), AddNPCAnswerAction);
		MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "AddLink", "Add Link"), NSLOCTEXT("PropertyView", "AddLink_ToolTip", "Usage: left click on the node you want to link"), FSlateIcon(), AddLinkAction);

		if (Owner->SelectedNodes.Num() == 1 && Owner->SelectedNodes[0] == Id)
		{
			MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "BreakLinks", "Break Links With Node..."), NSLOCTEXT("PropertyView", "BreakLinks_ToolTip", "Usage: left click on the node you want to break links with"), FSlateIcon(), BreakLinksModeAction);
		}

		MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "BreakOutLinks", "Break Outgoing Links"), NSLOCTEXT("PropertyView", "BreakOutLinks_ToolTip", "Breaks all outgoing links"), FSlateIcon(), BreakOutLinksAction);
		
		if (Dialogue->Data[NodeIndex].NodeOwner != EDialogueNodeOwner::DNO_EVENT)
		{
			MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "BreakInLinks", "Break Incoming Links"), NSLOCTEXT("PropertyView", "BreakInLinks_ToolTip", "Breaks all incoming links"), FSlateIcon(), BreakInLinksAction);
		}

		if (Id != 0)
		{
			if (Dialogue->Data[NodeIndex].NodeOwner == EDialogueNodeOwner::DNO_PLAYER)
			{
				MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "ChangeToNpc", "Change to NPC"), NSLOCTEXT("PropertyView", "ChangeToNpc_ToolTip", "Changes node to an NPC answer"), FSlateIcon(), ChangePcNpcAction);
			}
			else
			{
				MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "ChangeToPc", "Change to PC"), NSLOCTEXT("PropertyView", "ChangeToPc_ToolTip", "Changes node to a PC answer"), FSlateIcon(), ChangePcNpcAction);
			}
		}

		//can't delete start node
		if (Id != 0)	
		{
			MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "Delete", "Delete Node"), NSLOCTEXT("PropertyView", "Delete_ToolTip", "Deletes this node"), FSlateIcon(), DeleteAction);
		}

		if (Owner->SelectedNodes.IsValidIndex(0) && Owner->SelectedNodes.IsValidIndex(1)) // if multiple are selected
		{
			MenuBuilder.AddMenuEntry(NSLOCTEXT("PropertyView", "DeleteSelected", "Delete Selected Nodes"), NSLOCTEXT("PropertyView", "DeleteAll_ToolTip", "Deletes selected nodes"), FSlateIcon(), DeleteAllAction);
		}

		MenuBuilder.EndSection();

		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SDialogueNodeWidget::BreakLinksMode()
{
	Owner->bBreakingLinksMode = true;
	Owner->breakingLinksFromId = Id;
}

void SDialogueNodeWidget::BreakLinksWithNode()
{
	Owner->bBreakingLinksMode = false;

	// if the two nodes aren't linked in any way or we clicked on the same node, just return
	if (Owner->breakingLinksFromId == Id || (!Dialogue->Data[NodeIndex].Links.Contains(Owner->breakingLinksFromId) && !Owner->GetNodeById(Owner->breakingLinksFromId).Links.Contains(Id)))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("BreakLinksWithNode", "Break Links With Node..."));
	Dialogue->Modify();

	Dialogue->Data[NodeIndex].Links.RemoveAll([this](const int32 param1)
	{
		return (param1 == Owner->breakingLinksFromId);
	});

	int32 BreakingFromNodeIndex = Owner->NodeIdsIndexes.FindRef(Owner->breakingLinksFromId);

	Dialogue->Data[BreakingFromNodeIndex].Links.RemoveAll([this](const int32 param1)
	{
		return (param1 == Id);
	});
}

void SDialogueNodeWidget::OnChangePcNpc()
{
	if (Dialogue->Data[NodeIndex].NodeOwner == EDialogueNodeOwner::DNO_PLAYER)
	{
		const FScopedTransaction Transaction(LOCTEXT("ChangePcNpc", "Switch Node To NPC"));
		
		Dialogue->Data[NodeIndex].NodeOwner = EDialogueNodeOwner::DNO_NPC;

		FDialogueNode SampleNode;
		if (Dialogue->Data[NodeIndex].Links.IsValidIndex(0))
		{
			SampleNode = Dialogue->GetNodeById(Dialogue->Data[NodeIndex].Links[0]);
			if (SampleNode.NodeOwner == EDialogueNodeOwner::DNO_CHOICE) OnBreakOutLinks();
		}

		Dialogue->Modify();
	}
	else if (Dialogue->Data[NodeIndex].NodeOwner == EDialogueNodeOwner::DNO_NPC)
	{
		const FScopedTransaction Transaction(LOCTEXT("ChangeNpcPc", "Switch Node To PC"));
		Dialogue->Data[NodeIndex].NodeOwner = EDialogueNodeOwner::DNO_PLAYER;
		Dialogue->Modify();
	}

	//Dialogue->Data[NodeIndex].isPlayer = !Dialogue->Data[NodeIndex].isPlayer;
}

/* Called when deleting a single node.
* @param withRefresh - indicates wether a refresh is required after this node is deleted. You don't want to refresh when you're deleting multiple nodes with this method.
*/
void SDialogueNodeWidget::OnDeleteNode(bool withRefresh = true)
{
	if (Id == 0) return; //can't delete start node

	BreakInLinks();

	Dialogue->CurrentNodeId = -1;
	Dialogue->Data.RemoveAt(Owner->NodeIdsIndexes.FindRef(Id));
	Owner->DeselectNode(Id);

	if (withRefresh)
	{
		Owner->SpawnNodes();
	}
}

void SDialogueNodeWidget::OnBreakOutLinks()
{
	const FScopedTransaction Transaction(LOCTEXT("BreakOutLinks", "Break Outgoing Links"));
	Dialogue->Modify();

	Dialogue->Data[NodeIndex].Links.Empty();
}

void SDialogueNodeWidget::OnBreakInLinks()
{
	const FScopedTransaction Transaction(LOCTEXT("BreakInLinks", "Break Incoming Links"));
	Dialogue->Modify();
	BreakInLinks();
}

void SDialogueNodeWidget::BreakInLinks()
{
	int i = 0;
	for (auto Node : Dialogue->Data)
	{
		Dialogue->Data[i].Links.RemoveAll([this](const int32 param1)
		{
			return (param1 == Id);
		});
		i++;
	}	
}

void SDialogueNodeWidget::OnAddPcAnswer()
{
	const FScopedTransaction Transaction(LOCTEXT("AddPcAnswer", "Add PC Answer"));
	Dialogue->Modify();

	int index = NodeIndex;

	FDialogueNode NewNode;

	NewNode.id = Dialogue->NextNodeId;
	NewNode.NodeOwner = EDialogueNodeOwner::DNO_CHOICE;
	//NewNode.isPlayer = true; //since we use an enum for player

	if (Dialogue->Data[index].Links.Num() == 0)
	{
		NewNode.Coordinates.X = Dialogue->Data[index].Coordinates.X;
		NewNode.Coordinates.Y = Dialogue->Data[index].Coordinates.Y + (NodeSize.Y + Owner->NodeWidgets[0]->NodeSize.Y) / 2.f / Owner->GetZoomAmount() + 75.f; // half the size of current widget + half the size of first widget + desired distance
	}
	else
	{
		int32 lastIndex;
		int32 lastId;
		int32 linkToLastId = Dialogue->Data[index].Links.FindLastByPredicate([&](const int32 param1)
		{
			int tempIndex = Owner->NodeIdsIndexes.FindRef(param1);
			return Dialogue->Data[tempIndex].Coordinates.Y > Dialogue->Data[index].Coordinates.Y;
		});

		if (linkToLastId != INDEX_NONE)
		{
			lastId = Dialogue->Data[index].Links[linkToLastId];
			lastIndex = Owner->NodeIdsIndexes.FindRef(lastId);

			NewNode.Coordinates.X = Dialogue->Data[lastIndex].Coordinates.X + (Owner->NodeWidgets[lastIndex]->NodeSize.X + Owner->NodeWidgets[0]->NodeSize.X) / 2.f / Owner->GetZoomAmount() + 50.f; // half the size of the preceding widget + half the size of the new widget + desired distance
			NewNode.Coordinates.Y = Dialogue->Data[lastIndex].Coordinates.Y;
		}
		else
		{	// same as if (Dialogue->Data[index].Links.Num() == 0)
			NewNode.Coordinates.X = Dialogue->Data[index].Coordinates.X;
			NewNode.Coordinates.Y = Dialogue->Data[index].Coordinates.Y + (NodeSize.Y + Owner->NodeWidgets[0]->NodeSize.Y) / 2.f / Owner->GetZoomAmount() + 75.f; // half the size of current widget + half the size of first widget + desired distance
		}
	}

	Dialogue->NextNodeId++;
	Dialogue->Data.Add(NewNode);
	Dialogue->Data[index].Links.Add(NewNode.id);
	Owner->SpawnNodes(NewNode.id);
}

void SDialogueNodeWidget::OnAddTextEvent()
{
	//const FScopedTransaction Transaction(LOCTEXT("AddNpcAnswer", "Add NPC Answer"));
	const FScopedTransaction Transaction(LOCTEXT("AddTextEvent", "Add Text Event"));
	Dialogue->Modify();

	int index = NodeIndex;

	FDialogueNode NewNode;

	NewNode.id = Dialogue->NextNodeId;
	NewNode.NodeOwner = EDialogueNodeOwner::DNO_EVENT;
	//NewNode.isPlayer = false;

	if (Dialogue->Data[index].Links.Num() == 0)
	{
		NewNode.Coordinates.X = Dialogue->Data[index].Coordinates.X;
		NewNode.Coordinates.Y = Dialogue->Data[index].Coordinates.Y + (NodeSize.Y + Owner->NodeWidgets[0]->NodeSize.Y) / 2.f / Owner->GetZoomAmount() + 75.f; // half the size of current widget + half the size of first widget + desired distance
	}
	else
	{
		int32 lastIndex;
		int32 lastId; 
		int32 linkToLastId = Dialogue->Data[index].Links.FindLastByPredicate([&](const int32 param1)
		{
			int tempIndex = Owner->NodeIdsIndexes.FindRef(param1);
			return Dialogue->Data[tempIndex].Coordinates.Y > Dialogue->Data[index].Coordinates.Y;
		});

		if (linkToLastId != INDEX_NONE)
		{
			lastId = Dialogue->Data[index].Links[linkToLastId];
			lastIndex = Owner->NodeIdsIndexes.FindRef(lastId);

			NewNode.Coordinates.X = Dialogue->Data[lastIndex].Coordinates.X + (Owner->NodeWidgets[lastIndex]->NodeSize.X + Owner->NodeWidgets[0]->NodeSize.X) / 2.f / Owner->GetZoomAmount() + 50.f; // half the size of the preceding widget + half the size of the new widget + desired distance
			NewNode.Coordinates.Y = Dialogue->Data[lastIndex].Coordinates.Y;
		}
		else
		{	// same as if (Dialogue->Data[index].Links.Num() == 0)
			NewNode.Coordinates.X = Dialogue->Data[index].Coordinates.X;
			NewNode.Coordinates.Y = Dialogue->Data[index].Coordinates.Y + (NodeSize.Y + Owner->NodeWidgets[0]->NodeSize.Y) / 2.f / Owner->GetZoomAmount() + 75.f; // half the size of current widget + half the size of first widget + desired distance
		}
	}

	Dialogue->NextNodeId++;
	Dialogue->Data.Add(NewNode);
	//Dialogue->Data[index].Links.Add(NewNode.id); //on an event node, we don't need any links.
	Owner->SpawnNodes(NewNode.id);
}

void SDialogueNodeWidget::OnAddLink()
{
	Dialogue->isLinking = true;
	Owner->isLinkingAndCapturing = true;
	Dialogue->LinkingFromIndex = NodeIndex;
	Dialogue->LinkingCoords = Dialogue->Data[NodeIndex].Coordinates;
	Owner->ForceSlateToStayAwake();
}

/* After moving a node, we have to re-sort its parents child-links, so that the replies are shown based on the nodes horizontal positionning.
* For example, if Node A is to the left of Node B, then in-game you will see the answer A before answer B.
*/
void SDialogueNodeWidget::SortParentsLinks()
{
	int32 i = 0;
	for (auto Node : Dialogue->Data)
	{
		int32 j = 0;
		for (auto Link : Node.Links)
		{
			if (Dialogue->Data[i].Links[j] == Id) // if a link to current node has been found, then perform a sort on the links
			{
				Dialogue->Data[i].Links.Sort([&](const int32 id1, const int32 id2) // resort them depending on their x coordinate
				{
					int32 index1 = Owner->NodeIdsIndexes.FindRef(id1);
					int32 index2 = Owner->NodeIdsIndexes.FindRef(id2);

					return Dialogue->Data[index1].Coordinates.X < Dialogue->Data[index2].Coordinates.X;
				});
				break;
			}
			j++;
		}
		i++;
	}
}

#undef LOCTEXT_NAMESPACE
