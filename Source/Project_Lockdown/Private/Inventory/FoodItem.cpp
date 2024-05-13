// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FoodItem.h"


UFoodItem::UFoodItem()
{
	ItemType = It_Food;
	FoodType = FT_Dry;
}

void UFoodItem::Use(ACustomCharacter* Character)
{
	if (Character)
	{
		Character->Health += HealAmount;
	}
}
