// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagazineItem.h"
#include "Inventory/Item.h"
#include "WeaponItem.generated.h"

UENUM()
enum WeaponType
{
	WT_Melee		UMETA(DisplayName = "Melee"),
	WT_Primary		UMETA(DisplayName = "Primary"),
	WT_Secondary	UMETA(DisplayName = "Secondary"),
};

UCLASS()
class PROJECT_LOCKDOWN_API UWeaponItem : public UItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly , Category = "Weapon Properties") class UMagazineItem* ActiveMagazine;

	bool bIsBulletInChamber;
};
