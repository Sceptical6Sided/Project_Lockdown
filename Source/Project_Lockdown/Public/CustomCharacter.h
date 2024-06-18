// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

UCLASS()
class PROJECT_LOCKDOWN_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	//set Custom Movement Component as the movement component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement") class UCustomCharacterMovementComponent* CustomCharacterMovementComponent;
	
public:
	// Sets default values for this character's properties
	ACustomCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCameraComponent* CameraComponent;

	//Sets Inventory Component for init
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory") class UInventoryComponent* Inventory;

	//Sets Stat Component for init
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") class UStatsComponent* Stats;
	
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
	
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);
	
	virtual void Jump() override;
	virtual void StopJumping() override;
	
	FCollisionQueryParams GetIgnoreCharacterParams() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable) UStatsComponent* GetStatsComponent() const;

	//True while interacting with an item that has an interaction time
	bool IsInteracting() const;

	//Gets the time until we interact with the current interactable
	float GetRemainingInteractTime() const;
};
