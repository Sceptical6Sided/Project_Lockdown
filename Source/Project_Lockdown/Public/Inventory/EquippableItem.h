// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item.h"
#include "EquippableItem.generated.h"

UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Head UMETA(DisplayName = "Head"),
	EIS_Helmet UMETA(DisplayName = "Helmet"),
	EIS_Chest UMETA(DisplayName = "Chest"),
	EIS_Vest UMETA(DisplayName = "Vest"),
	EIS_Legs UMETA(DisplayName = "Legs"),
	EIS_Feet UMETA(DisplayName = "Feet"),
	EIS_Hands UMETA(DisplayName = "Hands"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_PrimaryWeapon UMETA(DisplayName = "Primary Weapon"),
	EIS_SecondaryWeapon UMETA(DisplayName = "Secondary Weapon"),
	EIS_MeleeWeapon UMETA(DisplayName = "MeleeWeapon"),
	EIS_Throwable UMETA(DisplayName = "Throwable Item"),
};

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class PROJECT_LOCKDOWN_API UEquippableItem : public UItem
{
	GENERATED_BODY()

public:

	UEquippableItem();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Equippables")
	EEquippableSlot Slot;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use(class ACustomCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category="Equippables")
	virtual bool Equip(class ACustomCharacter* Character);

	UFUNCTION(BlueprintCallable, Category="Equippables")
	virtual bool UnEquip(class ACustomCharacter* Character);

	virtual bool ShouldShowInInventory() const override;

	UFUNCTION(BlueprintPure, Category="Equippables")
	bool isEquipped() {return bEquipped;}

	//Server function to equip the item
	void setEquipped(bool bNewEquipped);
	
protected:
	UPROPERTY(ReplicatedUsing = EquipstatusChanged)
	bool bEquipped;

	UFUNCTION()
	void EquipStatusChanged();
};
