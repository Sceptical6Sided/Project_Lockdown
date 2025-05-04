// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Inventory/EquippableItem.h"
#include "CustomCharacter.generated.h"

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckedTime = 0.f;
		bInteractHeld = false;
	}
		
	//The current interactable component we're viewing, if there is one
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;

	//Time since we last checked for an interactable
	UPROPERTY()
	float LastInteractionCheckedTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

UCLASS()
class PROJECT_LOCKDOWN_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	// Sets default values for this character's properties
	ACustomCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCameraComponent* CameraComponent;

	//Sets Inventory Component for init
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory") class UInventoryComponent* Inventory;

	//Sets Stat Component for init
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") class UStatsComponent* Stats;

	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	TMap<EEquippableSlot, USkeletalMesh*> NakedMeshes;
	
	UPROPERTY(BlueprintReadOnly, Category = "Mesh")
	TMap<EEquippableSlot, USkeletalMeshComponent*> PlayerMeshes;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* HandsMesh;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* VestMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* BackpackMesh;

	bool bPressedCustomJump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.0))
	float Health = 100.f;

	// Called on Server to use item from the player inventory
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(class UItem* Item);

	//Called on Server to drop the item from the player inventory
	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(class UItem* Item, const int32 Quantity);

	//Needed because the pickups use a blueprint base class
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;
	
	virtual void Jump() override;
	virtual void StopJumping() override;
	
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const {return EquippedItems;}

protected:
	//set Custom Movement Component as the movement component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	class UCustomCharacterMovementComponent* CustomCharacterMovementComponent;
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Restart() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void SetActorHiddenInGame(bool bNewHidden) override;
public:

	UFUNCTION(BlueprintCallable)
	void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintPure, Category="Looting")
	bool IsLooting() const;

	UFUNCTION()
	void ItemAddedToInventory(class UItem* Item);

	UFUNCTION()
	void ItemRemovedFromInventory(class UItem* Item);

	
	bool EquipItem(class UEquippableItem* Item);
	bool UnEquipItem(class UEquippableItem* Item);

	void EquipGear(class UGearItem* Gear);
	void UnEquipGear(const EEquippableSlot Slot);

	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnEquippedItemsChanged OnEquippedItemsChanged;
	
	UFUNCTION(BlueprintPure)
	class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquippableSlot Slot);
	
protected:

	//Allows efficient access to equipped items
	UPROPERTY(VisibleAnywhere, Category="Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;
	
	//The time between checking for an interactable in seconds (set to 0.f for every tick)
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractionCheckFrequency;

	//Information about the current state of the interaction
	UPROPERTY()
	FInteractionData InteractionData;

	//How far we should be checking for an interactable via line-trace (in centimeters)
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	//Helper function to return the current interactable faster (return nullptr if not looking at an interactable)
	FORCEINLINE class UInteractionComponent* GetInteractable() const {return InteractionData.ViewedInteractionComponent;}

	FTimerHandle TimerHandle_Interact;
	
	void PerformInteractionCheck();

	void CantFindNewInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	
	UFUNCTION(BlueprintCallable) void BeginInteract();
	UFUNCTION(BlueprintCallable) void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();
	
	void Interact();

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable) UStatsComponent* GetStatsComponent() const;

	//True while interacting with an item that has an interaction time
	bool IsInteracting() const;

	//Gets the time until we interact with the current interactable
	float GetRemainingInteractTime() const;
};
