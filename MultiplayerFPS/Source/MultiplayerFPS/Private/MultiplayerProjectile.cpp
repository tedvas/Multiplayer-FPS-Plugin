// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerFPS.h"

// Sets default values
AMultiplayerProjectile::AMultiplayerProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;
	BulletWhizzingSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Bullet Whizzing Sound Component"));
	BulletWhizzingSoundComponent->SetupAttachment(ProjectileMesh, NAME_None);
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 10000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	ProjectileMesh->OnComponentHit.AddDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);

	RegisteredHit = false;

	bReplicates = true;
}

void AMultiplayerProjectile::OnProjectileMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RegisterHit(Hit);
}

void AMultiplayerProjectile::RegisterHit_Implementation(const FHitResult& Hit)
{
	if (RegisteredHit == false)
	{
		if (IsExplosive == true)
		{
			Explode();
			return;
		}

		AActor* HitActor = Hit.GetActor();

		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

		UParticleSystem* HitEffect;

		if (SurfaceType == SURFACE_HEAD || SurfaceType == SURFACE_CHEST || SurfaceType == SURFACE_TORSO || SurfaceType == SURFACE_LEG)
		{
			HitEffect = Blood;
		}
		else
		{
			HitEffect = DefaultHitEffect;
		}

		SpawnHitParticleEffect(HitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), Hit);

		if (SurfaceType == SURFACE_HEAD)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, HeadDamage, HitDirection, Hit, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);
		}
		else if (SurfaceType == SURFACE_TORSO)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, TorsoDamage, HitDirection, Hit, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);
		}
		else if (SurfaceType == SURFACE_LEG)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, LegDamage, HitDirection, Hit, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);
		}
		else
		{
			UGameplayStatics::ApplyPointDamage(HitActor, DefaultDamage, HitDirection, Hit, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);
		}

		Destroy();
	}

	RegisteredHit = true;
	ProjectileMesh->OnComponentHit.RemoveDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);
}

void AMultiplayerProjectile::Explode_Implementation()
{
	if (OwningPlayer && IsExplosive == true && DefaultDamage > 0 && ExplosiveDamageRadius > 0)
	{
		TArray<AActor*> IgnoredActors;

		if (ExplosionIgnoreOwner == true)
		{
			IgnoredActors.Add(OwningPlayer);
		}

		for (auto& Actor : ExplosionIgnoredActors)
		{
			if (Actor)
			{
				IgnoredActors.Add(Actor);
			}
		}

		for (auto& ActorClass : ExplosionIgnoredClasses)
		{
			if (ActorClass)
			{
				TArray<AActor*> Actors;

				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, Actors);

				for (auto& ActorToIgnore : Actors)
				{
					if (ActorToIgnore)
					{
						IgnoredActors.Add(ActorToIgnore);
					}
				}
			}
		}

		UGameplayStatics::ApplyRadialDamage(GetWorld(), DefaultDamage, GetActorLocation(), ExplosiveDamageRadius, DamageType, IgnoredActors, this, GetInstigatorController(), ExplosiveDoFullDamage, ECC_Visibility);

		if (DefaultHitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultHitEffect, GetActorLocation(), GetActorRotation(), ExplosionScale);
		}

		if (DefaultBulletHitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DefaultBulletHitSound, GetActorLocation());
		}
	}

	Destroy();

	RegisteredHit = true;
	ProjectileMesh->OnComponentHit.RemoveDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);
}

void AMultiplayerProjectile::SpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit)
{
	if (HasAuthority())
	{
		MulticastSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
	}
	else
	{
		ServerSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
	}
}

void AMultiplayerProjectile::ServerSpawnHitParticleEffect_Implementation(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit)
{
	MulticastSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
}

void AMultiplayerProjectile::MulticastSpawnHitParticleEffect_Implementation(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit)
{
	if (ParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleEffect, ImpactPoint);
	}

	USoundBase* ChosenHitSound;

	int HitSoundIndex;
	TArray<USoundBase*> HitSounds;
	TArray<UPhysicalMaterial*> HitPhysicalMaterials;

	BulletHitSounds.GenerateKeyArray(HitSounds);
	BulletHitSounds.GenerateValueArray(HitPhysicalMaterials);

	UPhysicalMaterial* HitPhysicalMaterial = Hit.PhysMaterial.Get();

	if (HitPhysicalMaterials.Contains(HitPhysicalMaterial))
	{
		HitSoundIndex = HitPhysicalMaterials.Find(HitPhysicalMaterial);

		if (HitSounds.IsValidIndex(HitSoundIndex))
		{
			ChosenHitSound = HitSounds[HitSoundIndex];
		}
		else
		{
			ChosenHitSound = DefaultBulletHitSound;
		}
	}
	else
	{
		ChosenHitSound = DefaultBulletHitSound;
	}

	if (ChosenHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChosenHitSound, ImpactPoint);
	}

	UMaterialInterface* ChosenDecal;

	int DecalIndex;
	TArray<UMaterialInterface*> DecalMaterials;
	TArray<UPhysicalMaterial*> DecalPhysicalMaterials;

	BulletHitDecals.GenerateKeyArray(DecalMaterials);
	BulletHitDecals.GenerateValueArray(DecalPhysicalMaterials);

	if (DecalPhysicalMaterials.Contains(HitPhysicalMaterial))
	{
		DecalIndex = DecalPhysicalMaterials.Find(HitPhysicalMaterial);

		if (DecalMaterials.IsValidIndex(DecalIndex))
		{
			ChosenDecal = DecalMaterials[DecalIndex];
		}
		else
		{
			ChosenDecal = DefaultBulletHitDecal;
		}
	}
	else
	{
		ChosenDecal = DefaultBulletHitDecal;
	}

	if (ChosenDecal)
	{
		UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ChosenDecal, BulletHitDecalSize, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
}

void AMultiplayerProjectile::SetOwningPlayer(APawn* NewOwningPlayer)
{
	if (NewOwningPlayer)
	{
		OwningPlayer = NewOwningPlayer;
	}
}

APawn* AMultiplayerProjectile::GetOwningPlayer()
{
	return OwningPlayer;
}

void AMultiplayerProjectile::SetIsExplosive(bool NewIsExplosive)
{
	IsExplosive = NewIsExplosive;
}

bool AMultiplayerProjectile::GetIsExplosive()
{
	return IsExplosive;
}

void AMultiplayerProjectile::SetExplosionScale(FVector NewExplosionScale)
{
	ExplosionScale = NewExplosionScale;
}

FVector AMultiplayerProjectile::GetExplosionScale()
{
	return ExplosionScale;
}

void AMultiplayerProjectile::SetExplosionIgnoredActors(TArray<AActor*> NewExplosionIgnoredActors)
{
	ExplosionIgnoredActors = NewExplosionIgnoredActors;
}

TArray<AActor*> AMultiplayerProjectile::GetExplosionIgnoredActors()
{
	return ExplosionIgnoredActors;
}

void AMultiplayerProjectile::SetExplosionIgnoredClasses(TArray<TSubclassOf<AActor>> NewExplosionIgnoredClasses)
{
	ExplosionIgnoredClasses = NewExplosionIgnoredClasses;
}

TArray<TSubclassOf<AActor>> AMultiplayerProjectile::GetExplosionIgnoredClasses()
{
	return ExplosionIgnoredClasses;
}

void AMultiplayerProjectile::SetDefaultDamage(float NewDefaultDamage)
{
	DefaultDamage = NewDefaultDamage;
}

float AMultiplayerProjectile::GetDefaultDamage()
{
	return DefaultDamage;
}

void AMultiplayerProjectile::SetHeadDamage(float NewHeadDamage)
{
	HeadDamage = NewHeadDamage;
}

float AMultiplayerProjectile::GetHeadDamage()
{
	return HeadDamage;
}

void AMultiplayerProjectile::SetTorsoDamage(float NewTorsoDamage)
{
	TorsoDamage = NewTorsoDamage;
}

float AMultiplayerProjectile::GetTorsoDamage()
{
	return TorsoDamage;
}

void AMultiplayerProjectile::SetLegDamage(float NewLegDamage)
{
	LegDamage = NewLegDamage;
}

float AMultiplayerProjectile::GetLegDamage()
{
	return LegDamage;
}

void AMultiplayerProjectile::SetExplosionIgnoreOwner(bool NewExplosionIgnoreOwner)
{
	ExplosionIgnoreOwner = NewExplosionIgnoreOwner;
}

bool AMultiplayerProjectile::GetExplosionIgnoreOwner()
{
	return ExplosionIgnoreOwner;
}

void AMultiplayerProjectile::SetExplosiveDamageRadius(float NewExplosiveDamageRadius)
{
	ExplosiveDamageRadius = NewExplosiveDamageRadius;
}

float AMultiplayerProjectile::GetExplosiveDamageRadius()
{
	return ExplosiveDamageRadius;
}

void AMultiplayerProjectile::SetExplosiveDoFullDamage(bool NewExplosiveDoFullDamage)
{
	ExplosiveDoFullDamage = NewExplosiveDoFullDamage;
}

bool AMultiplayerProjectile::GetExplosiveDoFullDamage()
{
	return ExplosiveDoFullDamage;
}

void AMultiplayerProjectile::SetHitDirection(FVector NewHitDirection)
{
	HitDirection = NewHitDirection;
}

FVector AMultiplayerProjectile::GetHitDirection()
{
	return HitDirection;
}

void AMultiplayerProjectile::SetBloodEffect(UParticleSystem* NewBloodEffect)
{
	Blood = NewBloodEffect;
}

UParticleSystem* AMultiplayerProjectile::GetBloodEffect()
{
	return Blood;
}

void AMultiplayerProjectile::SetDefaultHitEffect(UParticleSystem* NewDefaultHitEffect)
{
	DefaultHitEffect = NewDefaultHitEffect;
}

UParticleSystem* AMultiplayerProjectile::GetDefaultHitEffect()
{
	return DefaultHitEffect;
}

void AMultiplayerProjectile::SetDefaultBulletHitSound(USoundBase* NewDefaultBulletHitSound)
{
	DefaultBulletHitSound = NewDefaultBulletHitSound;
}

USoundBase* AMultiplayerProjectile::GetDefaultBulletHitSound()
{
	return DefaultBulletHitSound;
}

void AMultiplayerProjectile::SetBulletHitSounds(TMap<USoundBase*, UPhysicalMaterial*> NewBulletHitSounds)
{
	BulletHitSounds = NewBulletHitSounds;
}

TMap<USoundBase*, UPhysicalMaterial*> AMultiplayerProjectile::GetBulletHitSounds()
{
	return BulletHitSounds;
}

void AMultiplayerProjectile::SetDamageType(TSubclassOf<UDamageType> NewDamageType)
{
	DamageType = NewDamageType;
}

TSubclassOf<UDamageType> AMultiplayerProjectile::GetDamageType()
{
	return DamageType;
}

void AMultiplayerProjectile::SetDefaultBulletHitDecal(UMaterialInterface* NewBulletHitDecal)
{
	DefaultBulletHitDecal = NewBulletHitDecal;
}

UMaterialInterface* AMultiplayerProjectile::GetDefaultBulletHitDecal()
{
	return DefaultBulletHitDecal;
}

void AMultiplayerProjectile::SetBulletHitDecals(TMap<UMaterialInterface*, UPhysicalMaterial*> NewBulletHitDecals)
{
	BulletHitDecals = NewBulletHitDecals;
}

TMap<UMaterialInterface*, UPhysicalMaterial*> AMultiplayerProjectile::GetBulletHitDecals()
{
	return BulletHitDecals;
}

void AMultiplayerProjectile::SetBulletHitDecalSize(FVector NewBulletHitDecalSize)
{
	BulletHitDecalSize = NewBulletHitDecalSize;
}

FVector AMultiplayerProjectile::GetBulletHitDecalSize()
{
	return BulletHitDecalSize;
}

void AMultiplayerProjectile::SetWhizzingSoundVolumeBasedOnSpeed(bool NewWhizzingSoundVolumeBasedOnSpeed)
{
	WhizzingSoundVolumeBasedOnSpeed = NewWhizzingSoundVolumeBasedOnSpeed;
}

bool AMultiplayerProjectile::GetWhizzingSoundVolumeBasedOnSpeed()
{
	return WhizzingSoundVolumeBasedOnSpeed;
}

// Called when the game starts or when spawned
void AMultiplayerProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMultiplayerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BulletWhizzingSoundComponent && WhizzingSoundVolumeBasedOnSpeed == true)
	{
		float NewVolume = UKismetMathLibrary::VSize(ProjectileMesh->GetPhysicsLinearVelocity()) / ProjectileMovement->InitialSpeed;

		BulletWhizzingSoundComponent->SetVolumeMultiplier(NewVolume);
	}
}

void AMultiplayerProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerProjectile, OwningPlayer);
	DOREPLIFETIME(AMultiplayerProjectile, IsExplosive);
	DOREPLIFETIME(AMultiplayerProjectile, ExplosionScale);
	DOREPLIFETIME(AMultiplayerProjectile, RegisteredHit);
	DOREPLIFETIME(AMultiplayerProjectile, DefaultBulletHitSound);
}