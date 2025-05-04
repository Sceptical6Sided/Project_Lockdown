// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/GearItem.h"
#include "CustomCharacter.h"

UGearItem::UGearItem()
{
	
}

bool UGearItem::Equip(ACustomCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipGear(this);
	}
	
	return bEquipSuccessful;
}

bool UGearItem::UnEquip(ACustomCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);
	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipGear(Slot);
	}
	return bUnEquipSuccessful;
}
