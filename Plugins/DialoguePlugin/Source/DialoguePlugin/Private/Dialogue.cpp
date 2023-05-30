#include "Dialogue.h"
#include "Runtime/Core/Public/Misc/OutputDeviceNull.h"
#include "Engine/GameInstance.h"
#include "DialoguePluginPrivatePCH.h"

UDialogue::UDialogue(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UDialogue::AssignPersistentOuter(UGameInstance * inGameInstance)
{
	PersistentGameInstance = TWeakObjectPtr<UGameInstance>(inGameInstance);
}

void UDialogue::CleanOuter()
{
	PersistentGameInstance.Reset();
}

UWorld* UDialogue::GetWorld() const
{	
	if (PersistentGameInstance.IsValid())
	{
		return PersistentGameInstance.Get()->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

FDialogueNode UDialogue::GetNodeById(int32 id, int32 & index)
{
	index = -1;

	int i = 0;
	for (FDialogueNode FoundNode : Data)
	{
		if (FoundNode.id == id)
		{
			index = i;
			return FoundNode;
		}
		i++;
	}

	FDialogueNode Empty;
	return Empty;
}

FDialogueNode UDialogue::GetNodeById(int32 id)
{
	int32 index;
	return GetNodeById(id, index);
}

// added later for BP usage, not used in C++ by the plugin
FDialogueNode UDialogue::GetNodeById(int32 id, bool& found)
{
	int32 index;
	FDialogueNode result = GetNodeById(id, index);
	found = index != -1;
	return result;
}

FDialogueNode UDialogue::GetFirstNode()
{
	FDialogueNode StartNode = GetNodeById(0);

	if (StartNode.Links.Num() > 0)
	{
		return GetNodeById(StartNode.Links[0]);
	}

	FDialogueNode Empty;
	return Empty;
}

TArray<FDialogueNode> UDialogue::GetNextNodes(FDialogueNode Node, bool bOnlySampleNodes)
{
	//PA SPECIFIC CODE
	//process any events that need to be activated at the end of the node
	if (!bOnlySampleNodes
		&& !Node.bActivateEventAtStartOfNode
		&& Node.Events.Num() != 0)
	{

	}

	TArray<FDialogueNode> Output;

	//first get the nodes
	for (int32 foundindex : Node.Links)
	{
		Output.Add(GetNodeById(foundindex));
	}

	//PA SPECIFIC CODE
	//then process any events or conditions on them
	if (!bOnlySampleNodes)
	{
		for (FDialogueNode DNode : Output)
		{
			//TODO: are we going to add something here? Why did we make this?
		}
	}

	return Output;
}

//----------------------------------------------------------------------------------------------------
// PA SPECIFIC NODES START
//----------------------------------------------------------------------------------------------------

TArray<FDialogueNode> UDialogue::GetEventNodes(FName EventName)
{
	TArray<FDialogueNode> Output;

	for (FDialogueNode DNode : Data)
	{
		if (DNode.NodeOwner == EDialogueNodeOwner::DNO_EVENT
			&& DNode.TextEventName == EventName)
		{
			//Add the nodes connected to the event, not the event. Event has no text anyway.
			for (int32 foundindex : DNode.Links)
			{
				Output.Add(GetNodeById(foundindex));
			}
		}
	}

	return Output;
}

bool UDialogue::IsFinalNode(FDialogueNode Node, bool bFinalForCurrentSpeaker)
{
	if (bFinalForCurrentSpeaker)
	{
		for (int32 foundindex : Node.Links)
		{
			//does this work correctly? We need to doublecheck. 
			if (GetNodeById(Node.id).NodeOwner == EDialogueNodeOwner::DNO_PLAYER
				|| GetNodeById(Node.id).NodeOwner == EDialogueNodeOwner::DNO_CHOICE)
			{
				return GetNodeById(foundindex).NodeOwner == EDialogueNodeOwner::DNO_NPC;
			}
			else
			{
				//if it's not a player, it's an NPC
				return GetNodeById(foundindex).NPCIndex != GetNodeById(Node.id).NPCIndex;
			}
		}
	}
	else
	{
		return Node.Links.Num() == 0;
	}

	//no further links means it has to be the final node and that the conversation can be closed.
	return true;
}

AActor* UDialogue::GetNodeOwningActor(FDialogueNode Node)
{
	if (Node.NodeOwner == EDialogueNodeOwner::DNO_PLAYER || Node.NodeOwner == EDialogueNodeOwner::DNO_CHOICE)
		return Player;

	if (Node.NodeOwner == EDialogueNodeOwner::DNO_NPC
		&& NPCs.IsValidIndex(Node.NPCIndex))
	{
		return NPCs[Node.NPCIndex];
	}

	return NULL;
}

bool UDialogue::DialogueIsPlayerControlled()
{
	//If we have even one single instance where duration is 0 (i.e. infinite), we need to assign player control to it.
	for (FDialogueNode DNode : Data)
	{
		if (DNode.Duration == 0.f)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------
// PA SPECIFIC NODES END
//----------------------------------------------------------------------------------------------------

void UDialogue::CallFunctionByName(UObject* Object, FString FunctionName)
{
	FOutputDeviceNull ar;
	Object->CallFunctionByNameWithArguments(*FunctionName, ar, NULL, true);
}

UDialogueConditions::UDialogueConditions() 
{

}

UWorld* UDialogueConditions::GetWorld() const
{
	UDialogue * outerDialogue = GetTypedOuter<UDialogue>();

	if (outerDialogue)
	{
		return outerDialogue->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

UDialogueEvents::UDialogueEvents()
{

}

void UDialogueEvents::RecieveEventTriggered_Implementation(APlayerController* ConsideringPlayer, AActor* NPCActor)
{
	return;
}

bool UDialogueConditions::IsConditionMet_Implementation(APlayerController* ConsideringPlayer, AActor* NPCActor)
{
	return IsConditionMet_Internal(ConsideringPlayer, NPCActor);
}

UWorld* UDialogueEvents::GetWorld() const
{
	UDialogue * outerDialogue = GetTypedOuter<UDialogue>();

	if (outerDialogue)
	{
		return outerDialogue->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

#if WITH_EDITOR
/*
* If we add a comment to a node, but bDrawBubbleComment is false, set it to true so it's instantly displayed.
*/
void UDialogue::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.GetPropertyName().ToString().Equals(TEXT("BubbleComment"), ESearchCase::CaseSensitive))
	{
		if (CurrentNodeId < 0)
		{
			return;
		}
		int32 index;
		FDialogueNode selectedNodeCopy = GetNodeById(CurrentNodeId, index);		
		// if bDrawBubble is false, but BobbleComment contains text
		if (!selectedNodeCopy.bDrawBubbleComment && !selectedNodeCopy.BubbleComment.EqualTo(FText::GetEmpty()))
		{
			Data[index].bDrawBubbleComment = true;
		}
		// if bDrawBubble is true, but BobbleComment is empty
		if (selectedNodeCopy.bDrawBubbleComment && selectedNodeCopy.BubbleComment.EqualTo(FText::GetEmpty()))
		{
			Data[index].bDrawBubbleComment = false;
		}
	}
}
#endif