// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"

#pragma region SavedMove

UCustomCharacterMovementComponent::FSavedMove_Custom::FSavedMove_Custom()
{
	Saved_bWantsToSprint=0;
	Saved_bWantsToProne = 0;
	Saved_bWantsToWalk = 0;
	Saved_bPrevWantsToCrouch=0;
	Saved_bPressedCustomJump=0;
	Saved_bHadAnimRootMotion=0;
	Saved_bTransitionFinished=0;
}

bool UCustomCharacterMovementComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Custom* NewCustomMove = static_cast<FSavedMove_Custom*>(NewMove.Get());
	if(Saved_bWantsToSprint != NewCustomMove->Saved_bWantsToSprint)
	{
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;

	Saved_bWantsToProne = 0;
	Saved_bPrevWantsToCrouch = 0;

	Saved_bPressedCustomJump = 0;
	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;
}

uint8 UCustomCharacterMovementComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	uint8 Result = Super::FSavedMove_Character::GetCompressedFlags();

	if(Saved_bWantsToSprint) Result |= FLAG_Custom_0;
	//High-jacking the Jump flag to lighten network bandwidth
	if(Saved_bPressedCustomJump) Result |= FLAG_JumpPressed;

	return Result;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;

	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;

	Saved_bWantsToProne = CharacterMovement->Safe_bWantsToProne;

	Saved_bPressedCustomJump = CharacterMovement->CustomCharacterOwner->bPressedCustomJump;

	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;

	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;

	CharacterMovement->Safe_bWantsToProne = Saved_bWantsToProne;

	CharacterMovement->CustomCharacterOwner->bPressedCustomJump = Saved_bPressedCustomJump;

	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;
}

#pragma endregion

#pragma region Client Network Prediction Data
UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{ }

FSavedMovePtr UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}

#pragma endregion
