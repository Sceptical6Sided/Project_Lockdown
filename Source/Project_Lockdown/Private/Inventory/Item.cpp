// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Item.h"

AItem::AItem()
{
	Weight = 1.f;
	ItemDisplayName = FText::FromString("Item");
	UseActionText = FText::FromString("Use");
	ItemType = EItemType::IT_Junk;
	bIsThrowable = false;
	bIsSpecialItem=false;
}
