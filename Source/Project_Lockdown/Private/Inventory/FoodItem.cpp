// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FoodItem.h"

#define LOCTEXT_NAMESPACE "FoodItem"
UFoodItem::UFoodItem()
{
	ItemType = It_Food;
	FoodType = FT_Dry;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
	HealAmount = 20.f;
}

void UFoodItem::Use(ACustomCharacter* Character)
{
	if (Character)
	{
		Character->Health += HealAmount;
	}
}
#undef LOCTEXT_NAMESPACE