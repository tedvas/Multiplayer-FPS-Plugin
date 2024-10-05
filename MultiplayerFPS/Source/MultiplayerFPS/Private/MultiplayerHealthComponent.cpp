// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UMultiplayerHealthComponent::UMultiplayerHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Health = 100;
	MaxHealth = 100;
	AutoHealthRegen = true;
	TimeToStartHealthRegen = 5.0f;
	HealthRegenTimeBetweenTicks = 0.05f;
	AmountOfHealthRegenPerTick = 2;
}

void UMultiplayerHealthComponent::OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	GetWorld()->GetTimerManager().ClearTimer(StartHealthRegenTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenTickTimerHandle);

	int OldHealth = GetHealth();

	SetHealth(GetHealth() - Damage);

	OnTakeDamage.Broadcast(OldHealth, GetHealth());
}

void UMultiplayerHealthComponent::Die()
{
	OnDieUnreplicated.Broadcast();

	ServerDie();
}

void UMultiplayerHealthComponent::ServerDie_Implementation()
{
	MulticastDie();
}

void UMultiplayerHealthComponent::MulticastDie_Implementation()
{
	OnDie.Broadcast();
}

void UMultiplayerHealthComponent::StartHealthRegen()
{
	GetWorld()->GetTimerManager().ClearTimer(StartHealthRegenTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenTickTimerHandle);

	if (AutoHealthRegen == true)
	{
		HealthRegenTick();

		GetWorld()->GetTimerManager().SetTimer(HealthRegenTickTimerHandle, this, &UMultiplayerHealthComponent::HealthRegenTick, HealthRegenTimeBetweenTicks, true, HealthRegenTimeBetweenTicks);
	}
}

void UMultiplayerHealthComponent::HealthRegenTick()
{
	Health += AmountOfHealthRegenPerTick;

	if (Health >= MaxHealth)
	{
		GetWorld()->GetTimerManager().ClearTimer(StartHealthRegenTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(HealthRegenTickTimerHandle);

		Health = MaxHealth;
	}

	OnHealthChanged.Broadcast(Health, true);
}

void UMultiplayerHealthComponent::SetHealth(int NewHealth)
{
	Health = NewHealth;

	OnHealthChanged.Broadcast(NewHealth, false);

	if (Health <= 0)
	{
		Health = 0;

		Die();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(StartHealthRegenTimerHandle, this, &UMultiplayerHealthComponent::StartHealthRegen, TimeToStartHealthRegen, false, TimeToStartHealthRegen);
	}
}

int UMultiplayerHealthComponent::GetHealth()
{
	return Health;
}

bool UMultiplayerHealthComponent::GetIsDead()
{
	return GetHealth() <= 0;
}

void UMultiplayerHealthComponent::SetMaxHealth(int NewMaxHealth)
{
	MaxHealth = NewMaxHealth;

	if (MaxHealth < 0)
	{
		MaxHealth = 0;
	}
}

int UMultiplayerHealthComponent::GetMaxHealth()
{
	return MaxHealth;
}

void UMultiplayerHealthComponent::SetAutoHealthRegen(bool NewAutoHealthRegen)
{
	AutoHealthRegen = NewAutoHealthRegen;

	if (NewAutoHealthRegen == true)
	{
		if (Health < MaxHealth)
		{
			GetWorld()->GetTimerManager().SetTimer(StartHealthRegenTimerHandle, this, &UMultiplayerHealthComponent::StartHealthRegen, TimeToStartHealthRegen, false, TimeToStartHealthRegen);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(StartHealthRegenTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(HealthRegenTickTimerHandle);
	}
}

bool UMultiplayerHealthComponent::GetAutoHealthRegen()
{
	return AutoHealthRegen;
}

void UMultiplayerHealthComponent::SetTimeToStartHealthRegen(float NewTimeToStartHealthRegen)
{
	TimeToStartHealthRegen = NewTimeToStartHealthRegen;
}

float UMultiplayerHealthComponent::GetTimeToStartHealthRegen()
{
	return TimeToStartHealthRegen;
}

void UMultiplayerHealthComponent::SetHealthRegenTimeBetweenTicks(float NewHealthRegenTimeBetweenTicks)
{
	HealthRegenTimeBetweenTicks = NewHealthRegenTimeBetweenTicks;
}

float UMultiplayerHealthComponent::GetHealthRegenTimeBetweenTicks()
{
	return HealthRegenTimeBetweenTicks;
}

void UMultiplayerHealthComponent::SetAmountOfHealthRegenPerTick(int NewAmountOfHealthRegenPerTick)
{
	AmountOfHealthRegenPerTick = NewAmountOfHealthRegenPerTick;
}

int UMultiplayerHealthComponent::GetAmountOfHealthRegenPerTick()
{
	return AmountOfHealthRegenPerTick;
}

bool UMultiplayerHealthComponent::IsDead()
{
	if (Health <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UMultiplayerHealthComponent::SetOwningPlayerController(APlayerController* NewOwningPlayerController)
{
	OwningPlayerController = NewOwningPlayerController;
}

APlayerController* UMultiplayerHealthComponent::GetOwningPlayerController()
{
	return OwningPlayerController;
}

// Called when the game starts
void UMultiplayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwningActor = GetOwner();

		if (OwningActor)
		{
			GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UMultiplayerHealthComponent::OnDamaged);
		}
	}
}

// Called every frame
void UMultiplayerHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMultiplayerHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMultiplayerHealthComponent, OwningActor);
	DOREPLIFETIME(UMultiplayerHealthComponent, OwningPlayerController);
	DOREPLIFETIME(UMultiplayerHealthComponent, Health);
	DOREPLIFETIME(UMultiplayerHealthComponent, MaxHealth);
	DOREPLIFETIME(UMultiplayerHealthComponent, AutoHealthRegen);
	DOREPLIFETIME(UMultiplayerHealthComponent, TimeToStartHealthRegen);
	DOREPLIFETIME(UMultiplayerHealthComponent, HealthRegenTimeBetweenTicks);
	DOREPLIFETIME(UMultiplayerHealthComponent, AmountOfHealthRegenPerTick);
}