// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacter.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

//Called when inventory processed a change, UI will bind to this and refresh itself
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/**Called on server when an item is added to this inventory*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, class UItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, class UItem*, Item);

UENUM()
enum class EItemAddResult : uint8
{
	IAR_NoItemsAdded UMETA(DisplayName = "No items added"),
	IAR_SomeItemsAdded UMETA(DisplayName = "Some items added"),
	IAR_AllItemsAdded UMETA(DisplayName = "All items added")
};

//Represents the result of adding an item to the inventory
USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

public:

	FItemAddResult(){};
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), ActualAmountGiven(0) {};
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), ActualAmountGiven(InQuantityAdded) {};
	
	//The amount of the item that we tried to add
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive;

	//The amount of the item that was actually added in the end. (If we for example try adding 10 items but only 4 can be added due to reasons)
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountGiven; 

	//The result of the operation
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result;

	//if something went wrong we throw this ErrorText, used to display on the UI
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText;

#pragma region Helper Functions
	
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		AddedNoneResult.Result = EItemAddResult::IAR_NoItemsAdded;
		AddedNoneResult.ErrorText = ErrorText;
		
		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);
		AddedSomeResult.Result = EItemAddResult::IAR_SomeItemsAdded;
		AddedSomeResult.ErrorText = ErrorText;
		
		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddedAllResult(InItemQuantity);
		AddedAllResult.Result = EItemAddResult::IAR_AllItemsAdded;
		
		return AddedAllResult;
	}
#pragma endregion
	
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_LOCKDOWN_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItem;

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	/**
	 * Try adding an item to the inventory
	 * ErrorText the text to display if the item cannot be added to the inventory.
	 * the amount of the item that was added to the inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(class UItem* Item);

	/**
	 * Try adding an item to the inventory using the items class instead of an item instance
	 * ErrorText the text to display if the item cannot be added to the inventory.
	 * the amount of the item that was added to the inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	//Take some quantity away from the item, and remove it from the inventory when quantity reaches zero
	int32 ConsumeItem(class UItem* Item);
	int32 ConsumeItem(class UItem* Item, const int32 Quantity);

	//Remove item from the inventory returns true if successful
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(class UItem* Item);

	//Return true if we have a given amount of an item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<class UItem> ItemClass, const int32 Quantity = 1) const;

	//Return the first item with the same class as the given Item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItem(class UItem* Item) const;

	//Return the first item with the same class as the give ItemClass
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItemByClass(TSubclassOf<class UItem> ItemClass) const;

	//Get all inventory items that are a child of ItemClass.
	TArray<UItem*> FindItemsByClass(TSubclassOf<class UItem> ItemClass) const;
	
	UPROPERTY(Transient) UStatsComponent* StatsComponent;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacity(const int32 NewCapacity);
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const {return WeightCapacity;}

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const {return Capacity;}

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class UItem*> GetItems() const {return Items;}

protected:
	
	UPROPERTY(EditDefaultsOnly, Instanced) TArray<class UItem*> DefaultItems;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory") int32 Capacity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory") float WeightCapacity;
	UPROPERTY(ReplicatedUsing = OnRep_Items ,VisibleAnywhere, BlueprintReadOnly, Category = "Inventory") TArray<class UItem*> Items;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

private:
	UFUNCTION()
	void OnRep_Items();

	UPROPERTY()
	int32 ReplicatedItemsKey;

	FItemAddResult TryAddItem_Internal(class UItem* Item);

	//Don't call Items.Add() directly, this is a function that handles the networking, and it calls Items.Add() internally
	UItem* AddItem(class UItem* Item, const int32 Quantity);

	UFUNCTION()
	void ItemAdded(class UItem* Item);

	UFUNCTION()
	void ItemRemoved(class UItem* Item);
};
