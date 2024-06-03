// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "InteractableItem.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AInteractableItem : public AActor
{
	GENERATED_BODY()

public:
	AInteractableItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This compnent needs to be created in child classes in order to not be the root component"))
	UBoxComponent* PickupBoxCollision;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnPickupBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnPickupBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Interact(APawn* InteractingPlayer);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void Interact_BP(APawn* InteractingPlayer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool RunInteractOnServer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool MulticastInteract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool RunInteractOnClient;
};