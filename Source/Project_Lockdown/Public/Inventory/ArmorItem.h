// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/GearItem.h"
#include "ArmorItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_LOCKDOWN_API UArmorItem : public UGearItem
{
	GENERATED_BODY()
public:
	//Used when calculating damage done to the player
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Armor", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DamageDefenceMultiplier;
};
