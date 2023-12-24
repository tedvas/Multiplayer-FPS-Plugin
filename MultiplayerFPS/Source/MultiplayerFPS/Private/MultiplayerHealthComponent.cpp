// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UMultiplayerHealthComponent::UMultiplayerHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Health = 100;
	MaxHealth = 100;
}

void UMultiplayerHealthComponent::OnDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	Health -= Damage;

	OnHealthChanged.Broadcast(Health);

	if (Health <= 0)
	{
		Health = 0;

		Die();
	}
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

void UMultiplayerHealthComponent::SetHealth(int NewHealth)
{
	Health = NewHealth;

	if (Health <= 0)
	{
		Health = 0;

		Die();
	}
}

int UMultiplayerHealthComponent::GetHealth()
{
	return Health;
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
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "OwningActor Invalid MultiplayerHealthComponent.cpp:BeginPlay");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwner() Failed MultiplayerHealthComponent.cpp:BeginPlay");
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

	DOREPLIFETIME(UMultiplayerHealthComponent, Health);
	DOREPLIFETIME(UMultiplayerHealthComponent, MaxHealth);
	DOREPLIFETIME(UMultiplayerHealthComponent, OwningActor);
	DOREPLIFETIME(UMultiplayerHealthComponent, OwningPlayerController);
}