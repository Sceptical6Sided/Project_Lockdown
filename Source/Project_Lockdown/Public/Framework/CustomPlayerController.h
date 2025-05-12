// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_LOCKDOWN_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(Client,Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification(const FText& Message);
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDeathScreen(class ACustomCharacter* Killer);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLootMenu(const class UInventoryComponent* LootSource);
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowInGameUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideLootMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHitPlayer();
};
