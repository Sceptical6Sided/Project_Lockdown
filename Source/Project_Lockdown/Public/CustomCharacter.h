// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CustomCharacter.generated.h"

UCLASS()
class PROJECT_LOCKDOWN_API ACustomCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	//set Custom Movement Component as the movement component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Movement) class UCustomCharacterMovementComponent* CustomCharacterMovementComponent;

public:
	// Sets default values for this character's properties
	ACustomCharacter(const FObjectInitializer& ObjectInitializer);

	bool bPressedCustomJump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = 0.0))
	float Health = 100.f;

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

};
