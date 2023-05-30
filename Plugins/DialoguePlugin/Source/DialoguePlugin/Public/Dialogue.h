// Copyright Underflow Studios 2017
#pragma once
#include "Runtime/Engine/Classes/Engine/DataAsset.h"
#include "CoreMinimal.h"
#include "Dialogue.generated.h"

UENUM()
enum class EDialogueNodeOwner : uint8
{
	DNO_NPC UMETA(DisplayName = "NPC"),
	DNO_PLAYER UMETA(DisplayName = "Player"),
	DNO_CHOICE UMETA(DisplayName = "Choice"),
	DNO_EVENT UMETA(DisplayName = "Event")
};

UENUM()
enum class EDialogueNodeType : uint8
{
	//A normal text window
	DNT_NORMAL UMETA(DisplayName = "Normal"),
	//A scream text window
	DNT_SCREAM UMETA(DisplayName = "Scream"),
	//A thought text window
	DNT_THOUGHT UMETA(DisplayName = "Thought"),
	//A window with an animated image inside. Does not contain actual text.
	DNT_IMAGE UMETA(DisplayName = "Image"),
	//A text window without tail
	DNT_FLOATER UMETA(DisplayName = "Floater"),
	//A text without window that appears on darkened background. Like the monk-speak in Breath of the Wild.
	DNT_MYSTERY UMETA(DisplayName = "Mysterious")
};

class UDialogue;
// Extend this class to add custom events
UCLASS(Blueprintable, BlueprintType, abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DIALOGUEPLUGIN_API UDialogueEvents : public UObject
{
	GENERATED_BODY()
	
public:
	UDialogueEvents();

	//Called when the event is triggered. 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue Events")
	void RecieveEventTriggered(APlayerController* ConsideringPlayer, AActor* NPCActor);

	virtual class UWorld* GetWorld() const override;
};

// Extend this class to add custom conditions.
UCLASS(Blueprintable, BlueprintType, abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DIALOGUEPLUGIN_API UDialogueConditions : public UObject
{
	GENERATED_BODY()
	
public:
	UDialogueConditions();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue Conditions")
	bool IsConditionMet(APlayerController* ConsideringPlayer, AActor* NPCActor);

	UFUNCTION()
	virtual bool IsConditionMet_Internal(APlayerController* ConsideringPlayer, AActor* NPCActor) { return true; }

	virtual class UWorld* GetWorld() const override;
};

// A particular type of condition that nests normal conditions and returns true if at least one of them is true.
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DIALOGUEPLUGIN_API UOrCondition : public UDialogueConditions
{
	GENERATED_BODY()

public:
	UOrCondition(){ }

	UPROPERTY(Instanced, EditDefaultsOnly, Category = "Dialogue Conditions")
	TArray<UDialogueConditions*> OrConditions;

	virtual bool IsConditionMet_Internal(APlayerController* ConsideringPlayer, AActor* NPCActor) override
	{
		for (auto & cond : OrConditions)
		{
			if (cond->IsConditionMet(ConsideringPlayer, NPCActor))
				return true;
		}
		return false;
	}
};

// A particular type of condition that nests normal conditions and returns true if at least one of them is true.
UCLASS(NotBlueprintable, BlueprintType, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DIALOGUEPLUGIN_API UAndCondition : public UDialogueConditions
{
	GENERATED_BODY()

public:
	UAndCondition(){ }

	UPROPERTY(Instanced, EditDefaultsOnly, Category = "Dialogue Conditions")
	TArray<UDialogueConditions*> AndConditions;

	virtual bool IsConditionMet_Internal(APlayerController* ConsideringPlayer, AActor* NPCActor) override
	{
		for (auto & cond : AndConditions)
		{
			if (!cond->IsConditionMet(ConsideringPlayer, NPCActor))
				return false;
		}
		return true;
	}
};


USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		int32 id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		EDialogueNodeOwner NodeOwner = EDialogueNodeOwner::DNO_PLAYER;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		EDialogueNodeType NodeType = EDialogueNodeType::DNT_NORMAL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		FName TextEventName;

	//TextEventLinkers is a Map value, where the first value is the index of the string at which the event will be called, and the fname is the name of the event
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Node")
		TMap<int32, FName> TextEventLinkers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue Node")
		TMap<int32, float> TextSpeedLinkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		int32 NPCIndex = 0;
	
	//TODO: Phase this variable out, as we are using an enum for this stuff
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		bool isPlayer;

	//Todo: Make this more adjustable, allow variables in text. Allow text to be colored differently, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		FText Text;

	//Text after preprocessing has removed all the commands from the text. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		FText TextProcessed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		TArray<int32> Links;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		FVector2D Coordinates;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue Node")
		TArray<UDialogueEvents*> Events;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue Node", meta = (EditCondition = "Events.Num() != 0"))
		bool bActivateEventAtStartOfNode = true;

	UPROPERTY(Instanced, EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue Node")
		TArray<UDialogueConditions*> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		class USoundBase* Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		class UDialogueWave* DialogueWave = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node", meta = (MultiLine = "true"))
		FText BubbleComment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node")
		bool bDrawBubbleComment;

	//If set to true, the window will play the close animation after this node, even if it's not the last in the string of nodes. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Dialogue Node")
		bool bForceWindowClose = false;

	//Duration, if not 0, will cause the dialogue bubble to disappear/continue by itself if the bubble is not clicked away before then. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Node", meta = (EditCondition = "bIsTimed"))
		float Duration = 0.f;

	//GETTERS
	FText GetText() { return TextProcessed; };
	EDialogueNodeType GetType() { return NodeType; };
	EDialogueNodeOwner GetOwner() { return NodeOwner; };
	float GetDuration() { return Duration; };
};

class UGameInstance;

UCLASS(Blueprintable, BlueprintType)
class DIALOGUEPLUGIN_API UDialogue : public UDataAsset
{
	GENERATED_BODY()

public:

	UDialogue(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY()
		TArray<class AActor*> NPCs;

	UPROPERTY()
		class AActor* Player;
	
	/* Gets the first user-created node. If the dialogue only has the orange node, returns an empty node with -1 as Id */
	UFUNCTION(BlueprintCallable, Category = Dialogue)
	FDialogueNode GetFirstNode();

	UFUNCTION(BlueprintCallable, Category = Dialogue)
		TArray<FDialogueNode> GetNextNodes(FDialogueNode Node, bool bOnlySampleNodes = false);

	UFUNCTION(BlueprintCallable, Category = Dialogue)
		TArray<FDialogueNode> GetEventNodes(FName EventName);

	//CUSTOM ADDITION: checks whether it is the last node for that particular character, so we can close down that window and open a new one on input.
	//if bFinalForCurrentSpeaker is true, will only check for current speaker.
	UFUNCTION(BlueprintCallable, Category = Dialogue)
		bool IsFinalNode(FDialogueNode Node, bool bFinalForCurrentSpeaker = true);

	UFUNCTION(BlueprintCallable, Category = Dialogue)
	static void CallFunctionByName(UObject* Object, FString FunctionName);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dialogue)
	bool DisplayIdleSplines = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dialogue)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dialogue)
	TArray<FDialogueNode> Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dialogue)
	int32 NextNodeId;

	// changing the hard pointer to weakptr may fix the crash
	TWeakObjectPtr<UGameInstance> PersistentGameInstance;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AssignPersistentOuter(class UGameInstance * inGameInstance);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void CleanOuter();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
		AActor* GetNodeOwningActor(FDialogueNode Node);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
		bool DialogueIsPlayerControlled();

	virtual UWorld* GetWorld() const override;	

	FDialogueNode GetNodeById(int32 id, int32 &index);
	FDialogueNode GetNodeById(int32 id);
	UFUNCTION(BlueprintCallable, Category = Dialogue)
	FDialogueNode GetNodeById(int32 id, bool& found);

	int32 CurrentNodeId;
	bool isLinking;
	FVector2D LinkingCoords;
	int32 LinkingFromIndex;
	//FSlateImageBrush* bgStyle;
};
