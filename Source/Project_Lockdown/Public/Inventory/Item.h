// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacter.h"
#include "Item.generated.h"

UENUM()
enum EItemType
{
	IT_Junk			UMETA(DispalyName = "Junk"),
	IT_Food			UMETA(DisplayName = "Food"),
	IT_Armor		UMETA(DisplayName = "Armor"),
	IT_Weapon		UMETA(DispalyName = "Weapon"),
	IT_Magazine		UMETA(DisplayNme = "Magazine"),
	IT_Ammo			UMETA(DisplayName = "Ammo"),
	IT_Meds			UMETA(DispalyName = "Meds"),
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECT_LOCKDOWN_API AItem : public AActor
{
	GENERATED_BODY()
	
public:

	AItem();

	UPROPERTY(Transient) UWorld* World;

	//Used for tooltip
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") FText UseActionText;

	//Used for 3d representation in world
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") class UStaticMesh* PickUpMesh;

	//Used for categorizing Items
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item") TEnumAsByte<EItemType> ItemType;

	//Used for Item Thumbnail in Inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") UTexture2D* Thumbnail;

	//Used for item name for inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") FText ItemDisplayName;

	//Optional description for the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item", meta=(MultiLine = true)) FText ItemDescription;

	//Weight for the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item", meta=(ClampMin=0.0)) float Weight;

	//Indicator flag to know if the item is a "special item" or not, use for quest items and such
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item") bool bIsSpecialItem;

	//Inventory that owns the item
	UPROPERTY() class UInventoryComponent* OwningInventory;

#pragma region Throwing

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw", meta=(ClampMin=0.0)) float InitialSpeed = 800;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw", meta=(ClampMin=0.0)) float MaxSpeed = 8500;
	
	//Flag to tell the system if the item is throwable
	UPROPERTY(EditDefaultsOnly, Category="Throw")
	bool bIsThrowable;
	
#pragma endregion

	virtual void Use(class ACustomCharacter* Character) PURE_VIRTUAL(UItem,);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(ACustomCharacter* Character);
};
