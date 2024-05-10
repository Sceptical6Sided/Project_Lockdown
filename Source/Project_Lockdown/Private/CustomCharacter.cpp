// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomCharacter.h"
#include "CustomCharacterMovementComponent.h"
#include "StatsComponent.h"
#include "Camera/CameraComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Item.h"

//Helper Macro
#if 1
float MacroDuration = 2.f;
#define SLOG(x,...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, FString::Printf(TEXT(x), ##__VA_ARGS__));}
#else
#define SLOG(x,...)
#endif

// Sets default values
ACustomCharacter::ACustomCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CustomCharacterMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = true;


	/*Setup for modular character
	 *(Base mesh is used for the characters head,so
	 *every part of the mesh gets parented to the head of the character)
	 */
	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HelmetMesh");
	HelmetMesh->SetupAttachment(GetMesh());
	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ChestMesh");
	ChestMesh->SetupAttachment(GetMesh());
	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("LegsMesh");
	LegsMesh->SetupAttachment(GetMesh());
	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FeetMesh");
	FeetMesh->SetupAttachment(GetMesh());
	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandsMesh");
	HandsMesh->SetupAttachment(GetMesh());
	VestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("VestMesh");
	VestMesh->SetupAttachment(GetMesh());
	BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>("BackpackMesh");
	BackpackMesh->SetupAttachment(GetMesh());

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->Capacity = 20;
	
	Stats = CreateDefaultSubobject<UStatsComponent>("Stats");
	Inventory->StatsComponent = Stats;
}

void ACustomCharacter::UseItem(AItem* Item)
{
	if (Item)
	{
		Item->Use(this);
		Item->OnUse(this); //Bp event
	}
}

void ACustomCharacter::Jump()
{
	Super::Jump();

	bPressedCustomJump = true;

	bPressedJump = false;
}

void ACustomCharacter::StopJumping()
{
	Super::StopJumping();

	bPressedCustomJump = false;
}

//Getter function for StatsComponent
UStatsComponent* ACustomCharacter::GetStatsComponent() const
{
	return Stats;
}

//Returns an array of QueryParams mainly used when doing traces for collisions
FCollisionQueryParams ACustomCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

// Called when the game starts or when spawned
void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

