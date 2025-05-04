// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/EquippableItem.h"
#include "GearItem.generated.h"

UCLASS(Blueprintable)
class PROJECT_LOCKDOWN_API UGearItem : public UEquippableItem
{
	GENERATED_BODY()
public:
	UGearItem();

	virtual bool Equip(ACustomCharacter* Character) override;
	virtual bool UnEquip(ACustomCharacter* Character) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Gear")
	class USkeletalMesh* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Gear")
	class UMaterialInstance* MaterialInstance;
};
