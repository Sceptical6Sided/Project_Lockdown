// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class ACustomCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class ACustomCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class ACustomCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class ACustomCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class ACustomCharacter*, Character);

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_LOCKDOWN_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UInteractionComponent();

	//The time the player must hold the interact key to interact with this object
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTime;

	//The max distance the player can be away from this actor before you can interact (In centimeters)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance;

	//The name of the interactions target (for example: Chest, Lamp, Body... etc)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableNameText;

	//The verb describing the interaction (for example Loot, Open, Turn On/ Turn Off)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableActionText;

	//Whether this interactable is allowed to be used by multiple players at the same time 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bAllowMultipleUsers;

	//Call this to change the name of the Interactable at runtime
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetInteractableNameText(const FText& NewNameText);
	
	//Call this to change the action text of the Interactable at runtime
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void SetInteractableActionText(const FText& NewActionText);
	
	//Delegates

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	/*
	 *[local + server] Called when the player releases the interact key whilst focusing on this interactable actor,
	 *stops looking at this actor,
	 *or gets too far after starting the interaction
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	//[local + server] Called when the player starts looking at this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[local + server] Called when the player stops looking at this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the interactable actor for enough time
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;

protected:
	//Called at game start
	virtual void Deactivate() override;

	bool CanInteract(class ACustomCharacter* Character) const;

	//On the server, this will hold all the interacting characters. On the client this will just hold the local character (Provided they are interacting with this object)
	UPROPERTY()
	TArray<class ACustomCharacter*> Interacting_Characters;
	
public:

	//Called to refresh the interaction widget and its custom widgets
	void RefreshWidget();
	
	//Called on the client when the player's interaction check trace begins or ends hitting this item
	void BeginFocus(class ACustomCharacter* Character);
	void EndFocus(class ACustomCharacter* Character);

	//Called on the client when the player begins or ends the interaction with this item
	void BeginInteract(class ACustomCharacter* Character);
	void EndInteract(class ACustomCharacter* Character);
	
	void Interact(class ACustomCharacter* Character);

	//Return value from 0-1 denoting how far through the interaction the character is
	//On the server this is the first interacting character's percentage, on client this is the local character's percentage
	UFUNCTION(BlueprintPure, Category="Interaction")
	float GetInteractionPercentage();
};
