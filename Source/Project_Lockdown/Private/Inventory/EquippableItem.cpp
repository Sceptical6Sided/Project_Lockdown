// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/EquippableItem.h"
#include "Net/UnrealNetwork.h"
#include "CustomCharacter.h"

#define LOCTEXT_NAMESPACE "EquippableItem"

UEquippableItem::UEquippableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("ItemUseActionText","Equip");
}

void UEquippableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquippableItem, bEquipped);
}

void UEquippableItem::Use(ACustomCharacter* Character)
{
	if(Character && Character->HasAuthority())
	{
		//check to remove previously equipped item from the same slot
		if(Character -> GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquippableItem* AlreadyEquippedItem = *Character->GetEquippedItems().Find(Slot);
			AlreadyEquippedItem->setEquipped(false);
		}
		
		setEquipped(!isEquipped());
	}
}

bool UEquippableItem::Equip(ACustomCharacter* Character)
{
	if(Character)
	{
		return Character->EquipItem(this);
	}
	return false;
}

bool UEquippableItem::UnEquip(ACustomCharacter* Character)
{
	if(Character)
	{
		return Character->UnEquipItem(this);
	}
	return false;
}

bool UEquippableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquippableItem::setEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
	MarkDirtyForReplication();
}

void UEquippableItem::EquipStatusChanged()
{
	if (ACustomCharacter* Character = Cast<ACustomCharacter>(GetOuter()))
	{
		if(bEquipped)
		{
			Equip(Character);
		}
		else
		{
			UnEquip(Character);
		}
	}
	
	//Call delegate to update the UI
	OnItemModified.Broadcast();
}

#undef LOCTEXT_NAMESPACE
