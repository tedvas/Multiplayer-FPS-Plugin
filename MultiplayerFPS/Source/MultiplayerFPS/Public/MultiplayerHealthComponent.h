// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "MultiplayerHealthComponent.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, bool, IsRegenerating);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakeDamage, int, OldHealth, int, NewHealth);

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

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnTakeDamage OnTakeDamage;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerDie();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastDie();

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDie OnDie;

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnDieUnreplicated OnDieUnreplicated;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void StartHealthRegen();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void HealthRegenTick();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHealth(int NewHealth);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetHealth();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetMaxHealth(int NewMaxHealth);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetMaxHealth();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAutoHealthRegen(bool NewAutoHealthRegen);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetAutoHealthRegen();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetTimeToStartHealthRegen(float NewTimeToStartHealthRegen);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetTimeToStartHealthRegen();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHealthRegenTimeBetweenTicks(float NewHealthRegenTimeBetweenTicks);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetHealthRegenTimeBetweenTicks();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAmountOfHealthRegenPerTick(int NewAmountOfHealthRegenPerTick);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetAmountOfHealthRegenPerTick();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool IsDead();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetOwningPlayerController(APlayerController* NewOwningPlayerController);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual APlayerController* GetOwningPlayerController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0))
	int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0))
	int MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health")
	bool AutoHealthRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0.0f))
	float TimeToStartHealthRegen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0.0f))
	float HealthRegenTimeBetweenTicks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = 0))
	int AmountOfHealthRegenPerTick;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	AActor* OwningActor;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables", meta = (Tooltip = "Only applies when attached to players"))
	APlayerController* OwningPlayerController;

	FTimerHandle StartHealthRegenTimerHandle;
	FTimerHandle HealthRegenTickTimerHandle;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};