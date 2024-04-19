// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FoodItem.h"


AFoodItem::AFoodItem()
{
	ItemType = IT_Food;
	FoodType = FT_Dry;
}

void AFoodItem::Use(ACustomCharacter* Character)
{
	if (Character)
	{
		Character->Health += HealAmount;
	}
}
