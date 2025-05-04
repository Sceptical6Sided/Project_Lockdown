// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType)
enum EItemType
{
	It_Junk			UMETA(DisplayName = "Junk"),
	It_Clothing		UMETA(DisplayName = "Clothing"),
	It_Container	UMETA(DispalyName = "Container"),
	It_Food			UMETA(DisplayName = "Food"),
	It_Armor		UMETA(DisplayName = "Armor"),
	It_Weapon		UMETA(DisplayName = "Weapon"),
	It_Magazine		UMETA(DisplayNme = "Magazine"),
	It_Ammo			UMETA(DisplayName = "Ammo"),
	It_Meds			UMETA(DisplayName = "Meds"),
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECT_LOCKDOWN_API UItem : public UObject
{
	GENERATED_BODY()

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

	//Preprocessor marking for the engine to only keep this code for dev builds not for shipped versions
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:

	UItem();

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

	//Whether this item can be stacked
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") bool bStackable;

	//Maximum stack size if the item can be stacked
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item", meta = (ClampMin = 2, EditCondition = bStackable)) int32 MaxStackSize;
	
	//Weight for the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item", meta = (ClampMin = 0.0)) float Weight;

	//Indicator flag to know if the item is a "special item" or not, use for quest items and such
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item") bool bIsSpecialItem;
	
	//The tooltip widget for the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Item") TSubclassOf<class UItemTooltipWidget> ItemTooltip;

	//Server managed variable for the amount of the item
	UPROPERTY( ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category="Item", meta = (UIMin = 1, EditCondition = bStackable)) int32 Quantity;

	//Inventory that owns the item
	UPROPERTY() class UInventoryComponent* OwningInventory;

	//Used to efficiently replicate inventory items
	UPROPERTY() int32 RepKey;

	UPROPERTY(BlueprintAssignable) FOnItemModified OnItemModified;

	UFUNCTION()
	void OnRep_Quantity();

	UFUNCTION(BlueprintCallable, Category="Item") void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintCallable, Category="Item") FORCEINLINE int32 GetQuantity() const {return Quantity;}

	UFUNCTION(Blueprintable, Category="Item")
	FORCEINLINE bool IsStackFull() const { return Quantity >= MaxStackSize; }
	
	virtual void Use(class ACustomCharacter* Character) PURE_VIRTUAL(UItem,);

	virtual void AddedToInventory(class UInventoryComponent* Inventory);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUse(ACustomCharacter* Character);

	UFUNCTION(BlueprintCallable, Category="Item")
	FORCEINLINE float GetStackWeight() const {return Quantity * Weight; }

	UFUNCTION(BlueprintPure, Category="Item")
	virtual bool ShouldShowInInventory() const;

	//Mark the object as needing replication. We must call this internally after modifying any replicated properties
	void MarkDirtyForReplication();
};
