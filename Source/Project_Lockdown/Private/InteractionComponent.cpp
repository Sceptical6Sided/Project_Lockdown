// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

UInteractionComponent::UInteractionComponent()
{
	SetComponentTickEnabled(false);

	//Set base values
	InteractionTime = 0.f;
	InteractionDistance = 200.f;
	InteractableNameText = FText::FromString("Interactable");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleUsers = false;

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(400,100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true);
}

#pragma region Focus

void UInteractionComponent::BeginFocus(ACustomCharacter* Character)
{
	if(!IsActive() || !GetOwner() || !Character)
		return;

	OnBeginFocus.Broadcast(Character);
	
	SetHiddenInGame(false);

	//Handles outlines around interactable objects by setting render custom depth
	/*
	if(!GetOwner()->HasAuthority())
	{
		for (auto& VisualComp : GetOwner()->GetComponentByClass(UPrimitiveComponent::StaticClass()))
		{
			if(UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}
	}*/
}

void UInteractionComponent::EndFocus(ACustomCharacter* Character)
{
	
}

void UInteractionComponent::BeginInteract(ACustomCharacter* Character)
{
}

void UInteractionComponent::EndInteract(ACustomCharacter* Character)
{
}

void UInteractionComponent::Interact(ACustomCharacter* Character)
{
	OnInteract.Broadcast(Character);
}

#pragma endregion 
