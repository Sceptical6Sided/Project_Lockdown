// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item.h"
#include "CustomCharacter.h"
#include "FoodItem.generated.h"

UENUM()
enum EFoodType
{
	FT_Dry			UMETA(DisplayName="Dry"),
	FT_Wet			UMETA(DisplayName="Wet"),
	FT_Water		UMETA(DisplayName="Water"),
	FT_Soda			UMETA(DisplayName="Soda"),
	FT_Alcohol		UMETA(DisplayName="Alcohol")
};

UCLASS()
class PROJECT_LOCKDOWN_API AFoodItem : public AItem
{
	GENERATED_BODY()

public:

	AFoodItem();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Food Properties", meta = (ClampMin = 0.0)) float HealAmount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Food Properties") TEnumAsByte<EFoodType> FoodType;
	
protected:
	virtual void Use(ACustomCharacter* Character) override;
};
