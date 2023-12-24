// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerHealthComponent.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDie);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDieUnreplicated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERFPS_API UMultiplayerHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMultiplayerHealthComponent();

	UFUNCTION(BlueprintCallable)
	virtual void OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable)
	virtual void Die();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void ServerDie();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	virtual void MulticastDie();

	UPROPERTY(BlueprintAssignable)
	FOnDie OnDie;

	UPROPERTY(BlueprintAssignable)
	FOnDieUnreplicated OnDieUnreplicated;

	UFUNCTION(BlueprintCallable)
	virtual void SetHealth(int NewHealth);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int GetHealth();

	UFUNCTION(BlueprintCallable)
	virtual void SetMaxHealth(int NewMaxHealth);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int GetMaxHealth();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool IsDead();

	UFUNCTION(BlueprintCallable)
	virtual void SetOwningPlayerController(APlayerController* NewOwningPlayerController);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual APlayerController* GetOwningPlayerController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0))
	int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0))
	int MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	AActor* OwningActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, meta = (Tooltip = "Only applies when attached to players"))
	APlayerController* OwningPlayerController;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};