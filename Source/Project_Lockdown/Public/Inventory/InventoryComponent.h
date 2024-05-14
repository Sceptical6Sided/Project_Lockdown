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
	
	UPROPERTY(Transient) UStatsComponent* StatsComponent;

protected:
	
	UPROPERTY(EditDefaultsOnly, Instanced) TArray<class UItem*> DefaultItems;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory") int32 Capacity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory") float WeightCapacity;
	UPROPERTY(BlueprintAssignable, Category = "Inventory") FOnInventoryUpdated OnInventoryUpdated;
	UPROPERTY(ReplicatedUsing = OnRep_Items ,VisibleAnywhere, BlueprintReadOnly, Category = "Inventory") TArray<class UItem*> Items;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	
	bool AddItem(class UItem* Item);	
	bool RemoveItem(class UItem* Item);

private:
	UFUNCTION()
	void OnRep_Items();

	UPROPERTY()
	int32 ReplicatedItemsKey;
};
