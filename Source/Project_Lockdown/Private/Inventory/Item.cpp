// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Item.h"

#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItem, Quantity);
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

UItem::UItem()
{
	ItemDisplayName = LOCTEXT("ItemName","Item");
	UseActionText = LOCTEXT("ItemActionText","Use");
	Weight = 0.f;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;
	ItemType = EItemType::It_Junk;
	bIsSpecialItem=false;
}

void UItem::OnRep_Quantity()
{
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::MarkDirtyForReplication()
{
	
}

#undef LOCTEXT_NAMESPACE
