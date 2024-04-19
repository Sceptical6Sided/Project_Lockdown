// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Item.h"
#include "AmmoItem.generated.h"


UENUM()
enum EAmmoCalibre
{
	AC_Small			UMETA(DisplayName = "Small Arms"),
	AC_Medium			UMETA(DisplayName = "Intermediate"),
	AC_Large			UMETA(DisplayName = "Large Caliber"),
	AC_Special			UMETA(DisplayName = "Special Caliber")
};

UCLASS()
class PROJECT_LOCKDOWN_API AAmmoItem : public AItem
{
	GENERATED_BODY()
	
};
