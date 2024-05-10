// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomCharacter.h"
#include "CustomCharacterMovementComponent.h"
#include "InteractionComponent.h"
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

	InteractionCheckDistance = 1000.f;
	InteractionCheckFrequency = 0.f;

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

#pragma region Interaction

void ACustomCharacter::PerformInteractionCheck()
{
	if(GetController() == nullptr)
	{
		return;
	}

	InteractionData.LastInteractionCheckedTime = GetWorld()->GetTimeSeconds();
	
	FVector EyesLocation;
	FRotator EyesRotation;
	GetController()->GetPlayerViewPoint(EyesLocation, EyesRotation);
	
	FVector TraceEnd = (EyesRotation.Vector() * InteractionCheckDistance) + EyesLocation;
	FHitResult TraceHit;

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, EyesLocation, TraceEnd, ECC_Visibility, GetIgnoreCharacterParams()))
	{
		//Check if we hit an interactable object
		if(TraceHit.GetActor())
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float Distance = (EyesLocation - TraceHit.ImpactPoint).Size();

				if(InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent -> InteractionDistance && GetInteractable())
				{
					CantFindNewInteractable();
				}

				return;
			}
		}
	}

	CantFindNewInteractable();
}

void ACustomCharacter::CantFindNewInteractable()
{
	if (InteractionData.ViewedInteractionComponent)
	{
		InteractionData.ViewedInteractionComponent->SetHiddenInGame(true);
		InteractionData.ViewedInteractionComponent = nullptr;
	}
}

void ACustomCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	if(Interactable)
	{
		Interactable->SetHiddenInGame(false);
		InteractionData.ViewedInteractionComponent = Interactable;
	}
}

void ACustomCharacter::BeginInteract()
{
	if(!HasAuthority()) ServerBeginInteract();

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(this);

		if(FMath::IsNearlyZero(Interactable->InteractionTime))
			Interact();
		else
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ACustomCharacter::Interact, Interactable->InteractionTime, false);
	}
}

void ACustomCharacter::EndInteract()
{
	if(!HasAuthority()) ServerEndInteract();

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	if (UInteractionComponent* Interactable = GetInteractable())
		Interactable->EndInteract(this);
}

void ACustomCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool ACustomCharacter::ServerBeginInteract_Validate()
{
	return true;
}

void ACustomCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool ACustomCharacter::ServerEndInteract_Validate()
{
	return true;
}

void ACustomCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
		Interactable->Interact(this);
}

#pragma endregion

// Called when the game starts or when spawned
void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PerformInteractionCheck();
}

// Called to bind functionality to input
void ACustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

