// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_LOCKDOWN_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
//Health variables
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.0)) float Health = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.0)) float MaxHealth = 100.f;
private:
	UPROPERTY(EditDefaultsOnly, Category="Health") float HealthRegen = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Health") float HealthRegen_Rate = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category="Health") float HealthRegen_Delay = 1.f;
	
//Stamina variables
public:
	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Stamina", meta = (ClampMin = 0.0)) float MaxStamina = 100.f;
private:
	UPROPERTY(EditDefaultsOnly, Category="Stamina", meta = (ClampMin = 0.0), ReplicatedUsing="OnRep_Stamina") float Stamina = 100.f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina") float StaminaDecay = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina") float StaminaRegen = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina") float StaminaRegen_Rate = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category="Stamina") float StaminaRegen_Delay = 1.f;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void RegenStamina();
	
public:
	void DecayStamina();
	void StartRegenStamina();
	void StopRegenStamina();
	
	//Helper Functions for Stamina
	UFUNCTION()
	void OnRep_Stamina();
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStamina() const { return Stamina; }
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0 ? Stamina / MaxStamina : 0.f; }
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

private:
	FTimerHandle TimerHandle_StaminaRegen;
};
