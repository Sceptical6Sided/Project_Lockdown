// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

#include "Net/UnrealNetwork.h"

void UStatsComponent::RegenStamina()
{
	//value is clamped to avoid going over the MaxStamina rate
	Stamina = FMath::Clamp(Stamina + StaminaRegen, 0.0f, MaxStamina);
}

void UStatsComponent::DecayStamina()
{
	//value is clamped to avoid going into negatives
	Stamina = FMath::Clamp(Stamina + StaminaDecay, 0.0f, MaxStamina);
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
}
#pragma endregion 