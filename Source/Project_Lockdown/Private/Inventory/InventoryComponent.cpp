// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"

#include "Engine/ActorChannel.h"
#include "Inventory/Item.h"
#include "Net/UnrealNetwork.h"


#define LOCTEXT_NAMESPACE "InventoryComponent"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	OnItemAdded.AddDynamic(this, &UInventoryComponent::ItemAdded);
	OnItemRemoved.AddDynamic(this, &UInventoryComponent::ItemRemoved);
	
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//Check to see if we even need to replicate the channel
	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item: Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
			{
				bWroteSomething |= Channel -> ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}

	return bWroteSomething;
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();	
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);
			OnItemRemoved.Broadcast(Item);

			OnRep_Items();
			
			ReplicatedItemsKey++;
			
			return true;
		}
	}
	return false;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
	}
	
	return Weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::HasItem(TSubclassOf<UItem> ItemClass, const int32 Quantity) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(UItem* Item) const
{
	if (Item)
	{
		for (auto& InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
		//return FindItemByClass(Item->GetClass()); Alternate solution usage is TBD
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<UItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}
	
	return ItemsOfClass;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		//On the client the world won't be set initially, so it set if not
		if (!Item->World)
		{
			OnItemAdded.Broadcast(Item);
			Item->World = GetWorld();
		}
	}
}

int32 UInventoryComponent::ConsumeItem(UItem* Item)
{
	if(Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(UItem* Item, const int32 Quantity)
{
	if(GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		//Engine Boilerplate so we make sure that we don't have negative items after we consume 
		ensure(!(Item->GetQuantity()-RemoveQuantity < 0));

		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if(Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
		}
		else
		{
			ClientRefreshInventory();
		}

		return RemoveQuantity;
	}

	return 0;
}

#pragma region Adding Items

FItemAddResult UInventoryComponent::TryAddItem(UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<UItem> ItemClass, const int32 Quantity)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

UItem* UInventoryComponent::AddItem(UItem* Item, const int32 Quantity)
{
	if(GetOwner() && GetOwner()->HasAuthority())
	{
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->SetQuantity(Quantity);
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();
		OnItemAdded.Broadcast(NewItem);
		OnRep_Items();

		return NewItem;
	}
	return nullptr;
}

void UInventoryComponent::ItemAdded(UItem* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item added: %s on %s"), *GetNameSafe(Item), *RoleString);
}

void UInventoryComponent::ItemRemoved(UItem* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item Removed: %s on %s"), *GetNameSafe(Item), *RoleString);
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if(Item->bStackable)
		{
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			if(UItem* ExistingItem = FindItem(Item))
			{
				if(ExistingItem->IsStackFull())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackFullText", "Couldn't add {ItemName}. Tried adding items to a stack that was full."), Item->ItemDisplayName));
				}
				else
				{
					//Find the maximum amount of the item we could take due to weight
					const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
					const int32 QuantityMaxAddAmount = FMath::Min(ExistingItem->MaxStackSize - ExistingItem->GetQuantity(), Item->GetQuantity());
					const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);
					
					if (AddAmount <= 0)
					{
						//Already did full stack check, must not have enough weight
						return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->ItemDisplayName));
					}
					else
					{
						ExistingItem->SetQuantity(ExistingItem->GetQuantity() + AddAmount);
						return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
					}
				}
			}
			else
			{
				if (Items.Num() + 1 > GetCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->ItemDisplayName));
				}
				
				const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight); //What is the number of items we can add due to weight restriction
				const int32 QuantityMaxAddAmount = FMath::Min(Item->MaxStackSize, Item->GetQuantity());					//What is the number of items we can add due to StackSize restriction
				const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);							//What is the number of items we can actually add to the inventory (for ex, we can add x+1 due to weight, but StackSize is limited to x)

				AddItem(Item, AddAmount);

				return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
			}
		}
		else
		{
			if(Items.Num() + 1 > GetCapacity())
			{
				return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->ItemDisplayName));
			}

			//Items with zero weight don't require a weight check
			if (!FMath::IsNearlyZero(Item->Weight))
			{
				if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->ItemDisplayName));
				}
			}

			//Non-stackable items should always have a quantity of 1
			ensure(Item->GetQuantity() == 1);

			AddItem(Item, 1);

			return FItemAddResult::AddedAll(Item->GetQuantity());
		}
	}

	//This function should never be called on a client directly, or the network state would get compromised
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage","AddItem should never be called on a client"));
}

#pragma endregion

#undef LOCTEXT_NAMESPACE