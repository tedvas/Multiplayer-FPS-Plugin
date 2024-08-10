// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableItem.h"
#include "MultiplayerCharacter.h"

AInteractableItem::AInteractableItem()
{
	PickupBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Pickup Box Collision"));
	PickupBoxCollision->SetIsReplicated(true);

	PickupBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AInteractableItem::OnPickupBoxCollisionBeginOverlap);
	PickupBoxCollision->OnComponentEndOverlap.AddDynamic(this, &AInteractableItem::OnPickupBoxCollisionEndOverlap);

	RunInteractOnServer = false;
	MulticastInteract = false;
	RunInteractOnClient = false;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AInteractableItem::OnPickupBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OtherActor))
	{
		PlayerCast->SetOverlappingInteractable(true);
		PlayerCast->SetInteractableBeingOverlapped(this);
	}
}

void AInteractableItem::OnPickupBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OtherActor))
	{
		PlayerCast->SetOverlappingInteractable(false);
		PlayerCast->SetInteractableBeingOverlapped(nullptr);
	}
}

void AInteractableItem::Interact(APawn* InteractingPlayer)
{
	Interact_BP(InteractingPlayer);
	OnInteract.Broadcast(InteractingPlayer);
}