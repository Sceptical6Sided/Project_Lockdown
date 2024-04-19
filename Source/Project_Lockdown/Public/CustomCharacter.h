// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Inventory/Throwable.h"
#include "CustomCharacter.generated.h"

UCLASS()
class PROJECT_LOCKDOWN_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	//Sets Stat Component for init
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats") class UStatsComponent* Stats;
	
protected:
	//set Custom Movement Component as the movement component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement") class UCustomCharacterMovementComponent* CustomCharacterMovementComponent;

	//Sets Inventory Component for init
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory") class UInventoryComponent* Inventory;
	
public:
	// Sets default values for this character's properties
	ACustomCharacter(const FObjectInitializer& ObjectInitializer);

	bool bPressedCustomJump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.0)) float Health = 100.f;

	UPROPERTY(EditAnywhere) TSubclassOf<AThrowable> Throwable;
	
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class AItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Items")
	void ThrowItem(AItem* Item, USceneComponent* ThrowableSpawner);
	
	virtual void Jump() override;
	virtual void StopJumping() override;
	
	FCollisionQueryParams GetIgnoreCharacterParams() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable) UStatsComponent* GetStatsComponent() const;
};
