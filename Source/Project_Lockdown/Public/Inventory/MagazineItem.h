// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoItem.h"
#include "Inventory/Item.h"
#include "MagazineItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_LOCKDOWN_API UMagazineItem : public UItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Properties") uint8 BulletsLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Magazine Properties") TEnumAsByte<EAmmoCalibre> MagazineCaliber;
};
