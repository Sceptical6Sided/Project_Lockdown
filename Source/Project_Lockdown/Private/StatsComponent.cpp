// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

#include "Net/UnrealNetwork.h"

void UStatsComponent::RegenStamina()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetCurrentStamina(Stamina + StaminaRegen);
	}
	else
	{
		ServerSetCurrentStamina(Stamina + StaminaRegen);
	}
}

void UStatsComponent::ServerSetCurrentStamina_Implementation(float NewStamina)
{
	SetCurrentStamina(NewStamina);
}

void UStatsComponent::SetCurrentStamina(float NewStamina)
{
	//value is clamped to avoid going into negatives or going over the MaxStamina
	Stamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void UStatsComponent::DecayStamina()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetCurrentStamina(Stamina-StaminaDecay);
	}
	else
	{
		ServerSetCurrentStamina(Stamina-StaminaDecay);
	}
}

void UStatsComponent::StartRegenStamina()
{
	//Sanity check to see if there is a timer already to avoid starting a new one
	if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_StaminaRegen))
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_StaminaRegen,this, &UStatsComponent::RegenStamina, StaminaRegen_Rate, true, StaminaRegen_Delay);
	}
}

void UStatsComponent::StopRegenStamina()
{
	//Sanity check to see if there is a timer to clear
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_StaminaRegen))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StaminaRegen);
	}
}

void UStatsComponent::OnRep_Stamina()
{
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Broadcast from StatsComponent: %s"), *GetName());
}

#pragma region DefaultFunctions
// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	SetIsReplicatedByDefault(true);
}

void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatsComponent, Stamina);
}

// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] StatsComponent attached to: %s"), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] StatsComponent attached to: %s"), *GetOwner()->GetName());
	}
	
	OnStaminaChanged.Broadcast(Stamina,MaxStamina);
}
#pragma endregion 