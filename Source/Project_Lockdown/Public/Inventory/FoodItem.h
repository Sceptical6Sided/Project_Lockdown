// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item.h"
#include "CustomCharacter.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_LOCKDOWN_API AFoodItem : public AItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Food Properties", meta = (ClampMin = 0.0)) float HealAmount;
	
protected:
	virtual void Use(ACustomCharacter* Character) override;
};
