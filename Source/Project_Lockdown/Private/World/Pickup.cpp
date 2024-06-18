// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Pickup.h"

#include "InteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Inventory/Item.h"

// Sets default values
APickup::APickup()
{
	
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

		//Clients bind to this delegate on order to refresh the interaction widget if quantity changed
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

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if(PropertyName == GET_MEMBER_NAME_CHECKED(APickup, ItemTemplate))
	{
		PickupMesh->SetStaticMesh(ItemTemplate->PickUpMesh);
	}
}
#endif
