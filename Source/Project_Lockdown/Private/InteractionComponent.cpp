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
