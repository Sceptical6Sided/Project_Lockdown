// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Pickup.h"

#include "Inventory/Item.h"
#include "Net/UnrealNetwork.h"
#include "CustomCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/ActorChannel.h"

// Sets default values
APickup::APickup()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	SetRootComponent(PickupMesh);
	
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("PickupInteractionComponent");
	InteractionComponent->InteractionTime = 0.f;
	InteractionComponent->InteractionDistance = 200.f;
	InteractionComponent->InteractableNameText = FText::FromString("Pickup");
	InteractionComponent->InteractableActionText = FText::FromString("Take");
	InteractionComponent->OnInteract.AddDynamic(this, &APickup::OnTakePickup);
	InteractionComponent->SetupAttachment(PickupMesh);

	bReplicates = true;
}

void APickup::InitializePickup(const TSubclassOf<UItem> ItemClass, const int32 Quantity)
{
	if (HasAuthority() && ItemClass && Quantity > 0)
	{
		Item = NewObject<UItem>(this, ItemClass);
		Item->SetQuantity(Quantity);

		OnRep_Item();
		
		Item->MarkDirtyForReplication();
	}
}

void APickup::OnRep_Item()
{
	if(Item)
	{
		PickupMesh->SetStaticMesh(Item->PickUpMesh);
		InteractionComponent->InteractableNameText = Item->ItemDisplayName;

		//Clients bind to this delegate in order to refresh the interaction widget if quantity changed
		Item->OnItemModified.AddDynamic(this, &APickup::OnItemModified);
	}

	//If replicated properties changed,then refresh the widget
	InteractionComponent->RefreshWidget();
}

void APickup::OnItemModified()
{
	if(InteractionComponent)
	{
		InteractionComponent->RefreshWidget();
	}
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	//Check if the pickup is loaded from the map file directly and not spawned in at runtime
	if(HasAuthority() && ItemTemplate && bNetStartup)
	{
		InitializePickup(ItemTemplate->GetClass(), ItemTemplate->GetQuantity());
	}

	//If the pickup was spawned in at runtime align it with the ground
	if(!bNetStartup)
	{
		AlignWithGround();
	}

	if(Item)
	{
		Item->MarkDirtyForReplication();
	}
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, Item);
}

bool APickup::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (Item && Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
	{
		bWroteSomething |= Channel-> ReplicateSubobject(Item, *Bunch, *RepFlags);
	}
	
	return bWroteSomething;
}

#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//If a new mesh is set in the editor, change the mesh to reflect the change
	if(PropertyName == GET_MEMBER_NAME_CHECKED(APickup, ItemTemplate))
	{
		if(ItemTemplate)
		{
			PickupMesh->SetStaticMesh(ItemTemplate->PickUpMesh);	
		}
	}
}
#endif

void APickup::OnTakePickup(ACustomCharacter* Taker)
{
	if (!Taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup OnTakePickup was called but the player was not a valid Character."));
		return;
	}

	//Check PendingKillPending to prevent players taking a pickup that was already taken but the memory space was still occupied for it for some reason
	if(HasAuthority() && !IsPendingKillPending() && Item)
	{
		if(UInventoryComponent* Inventory = Taker->Inventory)
		{
			const FItemAddResult AddResult = Inventory->TryAddItem(Item);

			if(AddResult.ActualAmountGiven < Item->GetQuantity())
			{
				Item->SetQuantity(Item->GetQuantity()-AddResult.ActualAmountGiven);
			}
			else if (AddResult.ActualAmountGiven >= Item->GetQuantity())
			{
				Destroy();
			}
		}
	}
}
