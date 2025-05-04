// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "CustomCharacterMovementComponent.h"
#include "InteractionComponent.h"
#include "StatsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Inventory/GearItem.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Item.h"
#include "Materials/MaterialInstance.h"
#include "Windows/WindowsApplication.h"
#include "World/Pickup.h"

//Helper Macro
#if 0
float MacroDuration = 2.f;
#define SLOG(x,...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, FString::Printf(TEXT(x), ##__VA_ARGS__));}
#else
#define SLOG(x,...)
#endif

#define LOCTEXT_NAMESPACE "PlayerCharacter"

// Sets default values
ACustomCharacter::ACustomCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CustomCharacterMovementComponent = Cast<UCustomCharacterMovementComponent>(GetCharacterMovement());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = true;

	//Setting the meshes as members of the PlayerMeshes array, so they are more easily reachable, and the code is cleaner
	HelmetMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Helmet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh")));
	ChestMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Chest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh")));
	LegsMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	FeetMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Feet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	VestMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Vest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VestMesh")));
	HandsMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
	BackpackMesh=PlayerMeshes.Add(EEquippableSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));

	//Tell all the "body" meshes to use the head mesh for animation by replicating its pose component
	for (auto& PlayerMesh : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComponent = PlayerMesh.Value;
		MeshComponent->SetupAttachment(GetMesh());
		MeshComponent->SetLeaderPoseComponent(GetMesh());
	}
	
	//Finally adding the head to the array so it does not get attached to itself
	PlayerMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());
	
	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->SetCapacity(20);
	Inventory->SetWeightCapacity(80.f);
	Inventory->OnItemAdded.AddDynamic(this, &ACustomCharacter::ItemAddedToInventory);
	Inventory->OnItemRemoved.AddDynamic(this, &ACustomCharacter::ItemRemovedFromInventory);

	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;
	
	Stats = CreateDefaultSubobject<UStatsComponent>("Stats");
	Inventory->StatsComponent = Stats;

	SetReplicateMovement(true);
	SetReplicates(true);
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ACustomCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Caching out the default meshes for resets
	for (auto& PlayerMesh : PlayerMeshes)
	{
		NakedMeshes.Add(PlayerMesh.Key, PlayerMesh.Value->SkeletalMesh);
	}
	
	if (!IsLocallyControlled())
	{
		CameraComponent->DestroyComponent();
	}
}

// Called every frame
void ACustomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckedTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}

	if (IsLocallyControlled())
	{
		//Weapon handling code comes here
	}
}

void ACustomCharacter::UseItem(UItem* Item)
{
	if (!HasAuthority() && Item)
	{
		ServerUseItem(Item);
	}

	if (HasAuthority())
	{
		if (Inventory && !Inventory->FindItem(Item))
		{
			return;
		}
	}
	
	if (Item)
	{
		Item->Use(this);
		Item->OnUse(this); //Bp event
	}
}

void ACustomCharacter::ServerUseItem_Implementation(UItem* Item)
{
	UseItem(Item);
}

bool ACustomCharacter::ServerUseItem_Validate(UItem* Item)
{
	return true;
}

void ACustomCharacter::DropItem(UItem* Item, const int32 Quantity)
{
	if (Inventory && Item && Inventory->FindItem(Item))
	{
		if(!HasAuthority())
		{
			ServerDropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DroppedQuantity = Inventory->ConsumeItem(Item,Quantity);

			FActorSpawnParameters ItemSpawnParams;
			ItemSpawnParams.Owner = this;
			ItemSpawnParams.bNoFail = true;
			ItemSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector ItemSpawnLocation = GetActorLocation();
			ItemSpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform ItemSpawnTransform(GetActorRotation(), ItemSpawnLocation);

			ensure(PickupClass);

			APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, ItemSpawnTransform, ItemSpawnParams);
			Pickup-> InitializePickup(Item->GetClass(), DroppedQuantity);
		}
	}
}

void ACustomCharacter::ServerDropItem_Implementation(UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool ACustomCharacter::ServerDropItem_Validate(UItem* Item, const int32 Quantity)
{
	return true;
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

#pragma region Helper Functions

UStatsComponent* ACustomCharacter::GetStatsComponent() const
{
	return Stats;
}

bool ACustomCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ACustomCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
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

#pragma endregion

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
	//Lost focus on an interactable so we clear the timer
	if(GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	//Tell the interactable we've stopped focusing on it and clear the current interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
			EndInteract();
	}

	InteractionData.ViewedInteractionComponent = nullptr;
}

void ACustomCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	//Edge case, if the player started an interaction with one Interactable and focuses on a new Interactable
	EndInteract();

	if(UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable ->BeginFocus(this);
}

void ACustomCharacter::BeginInteract()
{
	if(!HasAuthority())
	{
		ServerBeginInteract();
	}

	if(HasAuthority())
	{
		PerformInteractionCheck();
	}
	
	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(this);

		if(FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ACustomCharacter::Interact, Interactable->InteractionTime, false);
		}
	}
}

void ACustomCharacter::EndInteract()
{
	if(!HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

void ACustomCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
		Interactable->Interact(this);
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

#pragma endregion

void ACustomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ACustomCharacter::Restart()
{
	Super::Restart();
}

float ACustomCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ACustomCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);
}

void ACustomCharacter::SetLootSource(UInventoryComponent* NewLootSource)
{
}

bool ACustomCharacter::IsLooting() const
{
	return false;
}

void ACustomCharacter::ItemAddedToInventory(UItem* Item)
{
}

void ACustomCharacter::ItemRemovedFromInventory(UItem* Item)
{
}

bool ACustomCharacter::EquipItem(UEquippableItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool ACustomCharacter::UnEquipItem(UEquippableItem* Item)
{
	if(Item)
	{
		if(EquippedItems.Contains(Item->Slot))
		{
			if(Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot,nullptr);
				return true;
			}
		}
	}
	return false;
}

void ACustomCharacter::EquipGear(class UGearItem* Gear)
{
	if(USkeletalMeshComponent* GearMesh = *PlayerMeshes.Find(Gear->Slot))
	{
		GearMesh->SetSkeletalMesh(Gear->Mesh);
		GearMesh->SetMaterial(GearMesh->GetMaterials().Num() -1, Gear->MaterialInstance);
	}
}

void ACustomCharacter::UnEquipGear(const EEquippableSlot Slot)
{
	if(USkeletalMeshComponent* EquippableMesh = *PlayerMeshes.Find(Slot))
	{
		if(USkeletalMesh* BodyMesh = * NakedMeshes.Find(Slot))
		{
			EquippableMesh->SetSkeletalMesh(BodyMesh);

			//Put the materials back on the body mesh
			for (int32 i = 0; i < BodyMesh->Materials.Num(); ++i)
			{
				if (BodyMesh->Materials.IsValidIndex(i))
				{
					EquippableMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
				}
			}
		}
		else
		{
			//set mesh to empty for external items, like backpacks
			EquippableMesh->SetSkeletalMesh(nullptr);
		}
	}
}

USkeletalMeshComponent* ACustomCharacter::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if(PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

// Called to bind functionality to input
void ACustomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

#undef LOCTEXT_NAMESPACE