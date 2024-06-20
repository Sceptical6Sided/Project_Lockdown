// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS(ClassGroup = (Items), Blueprintable, Abstract)
class PROJECT_LOCKDOWN_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	//Takes the item and creates a pickup for it (done on BeginPlay when a player drops an item on the ground)
	void InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	//Align pickup objects rotation with ground rotation
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();
	
	//Used as a template to create the pickup when spawned in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UItem* ItemTemplate;
	
protected:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, ReplicatedUsing = OnRep_Item)
	class UItem* Item;

	UFUNCTION()
	void OnRep_Item();

	UFUNCTION()
	void OnItemModified();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	//Called when a player takes the pickup object
	UFUNCTION()
	void OnTakePickup(class ACustomCharacter* Taker);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	class UInteractionComponent* InteractionComponent;
};
