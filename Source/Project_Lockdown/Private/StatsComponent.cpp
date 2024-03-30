// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"

// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{

}


// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatsComponent::DecayStamina()
{
	Stamina -= StaminaDecay;
}

void UStatsComponent::StartRegenStamina()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_StaminaRegen,this, &UStatsComponent::RegenStamina, StaminaRegen_Rate, true, StaminaRegen_Delay);
}

void UStatsComponent::StopRegenStamina()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StaminaRegen);
}

