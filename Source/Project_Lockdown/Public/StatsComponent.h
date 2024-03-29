// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_LOCKDOWN_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();
	
//Stamina variables
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Stamina") float Stamina = 100.f;
private:
	UPROPERTY(EditDefaultsOnly,Category="Stamina") float StaminaDecay = 0.5f;
	UPROPERTY(EditDefaultsOnly,Category="Stamina") float StaminaRegen = 0.5f;
	UPROPERTY(EditDefaultsOnly,Category="Stamina") float StaminaRegen_Rate = 0.25f;
	UPROPERTY(EditDefaultsOnly,Category="Stamina") float StaminaRegen_Delay = 1.f;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
