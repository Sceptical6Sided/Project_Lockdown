// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacter.h"
#include "StatsComponent.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

//Bp will bind to this to update UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_LOCKDOWN_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly, Instanced) TArray<class UItem*> DefaultItems;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory") int32 Capacity;
	UPROPERTY(BlueprintAssignable, Category = "Inventory") FOnInventoryUpdated OnInventoryUpdated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory") TArray<class UItem*> Items;
	UPROPERTY(Transient) UStatsComponent* StatsComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	
	bool AddItem(class UItem* Item);	
	bool RemoveItem(class UItem* Item);	
};
