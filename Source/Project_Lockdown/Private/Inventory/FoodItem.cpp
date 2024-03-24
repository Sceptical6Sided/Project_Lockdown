// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FoodItem.h"


void UFoodItem::Use(ACustomCharacter* Character)
{
	if (Character)
	{
		Character->Health += HealAmount;
	}
}
