// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

#include "CustomCharacter.h"

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

void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	
}

void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
}

float UInteractionComponent::GetInteractionPercentage()
{
	if (Interacting_Characters.IsValidIndex(0))
	{
		if (ACustomCharacter* Interacting_Character = Interacting_Characters[0])
		{
			if(Interacting_Character && Interacting_Character->IsInteracting())
				return 1.f - FMath::Abs(Interacting_Character->GetRemainingInteractTime() / InteractionTime);
		}
	}
	return 0.f;
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interacting_Characters.Num() - 1; i >= 0; --i)
	{
		if (ACustomCharacter* Interacting_Character = Interacting_Characters[i])
		{
			EndFocus(Interacting_Character);
			EndInteract(Interacting_Character);
		}
	}

	Interacting_Characters.Empty();
}

bool UInteractionComponent::CanInteract(ACustomCharacter* Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleUsers && Interacting_Characters.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponent::RefreshWidget()
{
}

#pragma region Focus

void UInteractionComponent::BeginFocus(ACustomCharacter* Character)
{
	if(!IsActive() || !GetOwner() || !Character)
		return;

	OnBeginFocus.Broadcast(Character);
	
	SetHiddenInGame(false);
}

void UInteractionComponent::EndFocus(ACustomCharacter* Character)
{
	OnEndFocus.Broadcast(Character);
	SetHiddenInGame(true);
}

void UInteractionComponent::BeginInteract(ACustomCharacter* Character)
{
	if(CanInteract(Character))
	{
		Interacting_Characters.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UInteractionComponent::EndInteract(ACustomCharacter* Character)
{
	Interacting_Characters.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UInteractionComponent::Interact(ACustomCharacter* Character)
{
	if(CanInteract(Character))
		OnInteract.Broadcast(Character);
}

#pragma endregion 
