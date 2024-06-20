// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

#include "CustomCharacter.h"
#include "Widgets/InteractionWidget.h"

#if 0
float MacroDuration = 2.f;
#define SLOG(x,...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, FString::Printf(TEXT(x), ##__VA_ARGS__));}
#else
#define SLOG(x,...)
#endif

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
	DrawSize = FIntPoint(600,100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true);
}

#pragma region Focus

void UInteractionComponent::BeginFocus(ACustomCharacter* Character)
{
	if(!IsActive() || !GetOwner() || !Character)
	{
		return;
	}

	OnBeginFocus.Broadcast(Character);

	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(false);	
	}

	RefreshWidget();
}

void UInteractionComponent::EndFocus(ACustomCharacter* Character)
{
	OnEndFocus.Broadcast(Character);
	if(GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(true);	
	}
}
#pragma endregion

#pragma region Delegate Broadcasting

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
	{
		OnInteract.Broadcast(Character);
	}
}

#pragma endregion

#pragma region Widget Interface

void UInteractionComponent::RefreshWidget()
{
	//Make sure the widget is initialized, and that we are displaying the right values (these may have changed)
	if(!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
			InteractionWidget->UpdateInteractionWidget(this);
	}
}

#pragma endregion

#pragma region Helper Functions

void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
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

#pragma endregion