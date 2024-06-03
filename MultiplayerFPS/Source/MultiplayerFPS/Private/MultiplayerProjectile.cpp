// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "MultiplayerCharacter.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine.h"

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
	ProjectileMovement->InitialSpeed = 50000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial Force Component"));
	RadialForceComponent->SetupAttachment(ProjectileMesh, NAME_None);

	ProjectileMesh->OnComponentHit.AddDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);

	LaunchPhysicsObjects = false;
	LaunchObjectStrength = 7500.0f;
	LaunchObjectVelocityChange = false;
	IsExplosive = false;
	ExplosionIgnoreOwner = false;
	DefaultDamage = 35.0f;
	ExplosiveDamageRadius = 500.0f;
	ExplosiveDoFullDamage = false;
	BulletHitMode = 0;
	BulletHitModeDelay = 0.0f;
	HitEffectScale = FVector(1.0f, 1.0f, 1.0f);
	CanCrumbleDestructibleMeshes = true;
	DestructionSphereSize = FVector(0.25f, 0.25f, 0.25f);

	RegisteredHit = false;

	bReplicates = true;
}

void AMultiplayerProjectile::DestroySelf()
{
	Destroy();
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
			Explode(Hit);
			return;
		}

		AActor* HitActor = Hit.GetActor();
		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
		UParticleSystem* HitEffect;
		UPhysicalMaterial* HitSurface = Hit.PhysMaterial.Get();

		FOnProjectileHit.Broadcast(HitActor, HitSurface);

		if (OwningPlayer)
		{
			if (HitActor == OwningPlayer)
			{
				return;
			}

			if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
			{
				if (AMultiplayerCharacter* EnemyCast = Cast<AMultiplayerCharacter>(HitActor))
				{
					if (EnemyCast->GetHealthComponent())
					{
						if (EnemyCast->GetHealthComponent()->GetHealth() > 0)
						{
							if (UseActorClassesForHitMarkers == 0 && HitActor)
							{
								PlayerCast->ShowHitMarker(HitActor, nullptr);
							}
							else if (UseActorClassesForHitMarkers == 1 && HitSurface)
							{
								PlayerCast->ShowHitMarker(nullptr, HitSurface);
							}
							else if (UseActorClassesForHitMarkers == 2 && HitActor && HitSurface)
							{
								PlayerCast->ShowHitMarker(HitActor, HitSurface);
							}
						}
					}
				}
			}
		}

		TArray<UPhysicalMaterial*> HitEffectPhysicalMaterials;
		TArray<UParticleSystem*> HitParticleSystems;

		HitEffects.GenerateKeyArray(HitEffectPhysicalMaterials);
		HitEffects.GenerateValueArray(HitParticleSystems);

		if (HitEffectPhysicalMaterials.Contains(HitSurface))
		{
			int HitEffectIndex;
			HitEffectIndex = HitEffectPhysicalMaterials.Find(HitSurface);

			if (HitParticleSystems.IsValidIndex(HitEffectIndex))
			{
				HitEffect = HitParticleSystems[HitEffectIndex];
			}
			else
			{
				HitEffect = DefaultHitEffect;
			}
		}
		else
		{
			HitEffect = DefaultHitEffect;
		}

		SpawnHitParticleEffect(HitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), Hit);

		float DamageToApply;

		TArray<UPhysicalMaterial*> DamagePhysicalMaterials;
		TArray<float> DamageAmounts;

		Damage.GenerateKeyArray(DamagePhysicalMaterials);
		Damage.GenerateValueArray(DamageAmounts);

		if (DamagePhysicalMaterials.Contains(HitSurface))
		{
			int DamageIndex;
			DamageIndex = DamagePhysicalMaterials.Find(HitSurface);

			if (DamageAmounts.IsValidIndex(DamageIndex))
			{
				DamageToApply = DamageAmounts[DamageIndex];
			}
			else
			{
				DamageToApply = DefaultDamage;
			}
		}
		else
		{
			DamageToApply = DefaultDamage;
		}

		if (LaunchPhysicsObjects == true && LaunchObjectStrength > 0)
		{
			if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
			{
				if (HitComponent->IsSimulatingPhysics() == true)
				{
					HitComponent->AddImpulse(HitDirection * LaunchObjectStrength, NAME_None, LaunchObjectVelocityChange);
				}
			}
		}

		if (BulletHitMode != 2)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, HitDirection, Hit, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);
		}

		if (BulletHitMode != 0 && BulletHitModeDelay <= 0.0f)
		{
			ExecuteHitFunction(OwningPlayer, HitActor);
		}

		RegisteredHit = true;

		MulticastSilenceBulletWhizzingSound();

		if (BulletHitMode == 0 || BulletHitModeDelay <= 0.0f)
		{
			DestroySelf();
		}

		if (DestructionSphereToSpawn && CanCrumbleDestructibleMeshes == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			GetWorld()->SpawnActor<AActor>(DestructionSphereToSpawn, FTransform(Hit.ImpactPoint.Rotation(), Hit.ImpactPoint + (Hit.ImpactNormal * -3.0f), DestructionSphereSize), SpawnParams);
		}

		MulticastSoftDestroyProjectile();
	}

	ProjectileMesh->OnComponentHit.RemoveDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);
}

void AMultiplayerProjectile::Explode_Implementation(const FHitResult& Hit, bool UseCurrentLocationForHit)
{
	if (OwningPlayer && IsExplosive == true && DefaultDamage > 0 && ExplosiveDamageRadius > 0)
	{
		FVector HitLocation;

		if (UseCurrentLocationForHit == true)
		{
			HitLocation = GetActorLocation();
		}
		else
		{
			HitLocation = Hit.ImpactPoint;
		}
		
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

		if (RadialForceComponent && LaunchPhysicsObjects == true && LaunchObjectStrength > 0)
		{
			RadialForceComponent->FireImpulse();
		}

		if (BulletHitMode != 2)
		{
			if (UGameplayStatics::ApplyRadialDamage(GetWorld(), DefaultDamage, GetActorLocation(), ExplosiveDamageRadius, DamageType, IgnoredActors, this, GetInstigatorController(), ExplosiveDoFullDamage, ExplosiveCollisionChannel))
			{
				FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage),  false, nullptr);

				SphereParams.AddIgnoredActors(IgnoredActors);

				TArray<FOverlapResult> Overlaps;
				if (UWorld* World = GEngine->GetWorldFromContextObject(GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
				{
					World->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(ExplosiveDamageRadius), SphereParams);
				}

				TMap<AActor*, TArray<FHitResult>> OverlapComponentMap;
				for (const FOverlapResult& Overlap : Overlaps)
				{
					AActor* const OverlapActor = Overlap.OverlapObjectHandle.FetchActor();

					if (OverlapActor &&
						OverlapActor->CanBeDamaged() &&
						Overlap.Component.IsValid())
					{
						FCollisionQueryParams LineParams(SCENE_QUERY_STAT(ComponentIsVisibleFrom), true, nullptr);
						LineParams.AddIgnoredActors( IgnoredActors );

						TWeakObjectPtr<UPrimitiveComponent> VictimComp = Overlap.Component;

						UWorld* const World = VictimComp->GetWorld();
						check(World);

						FVector const TraceEnd = VictimComp->Bounds.Origin;
						FVector TraceStart = GetActorLocation();
						if (GetActorLocation() == TraceEnd)
						{
							TraceStart.Z += 0.01f;
						}

						FHitResult OutHitResult;
						
						FVector const FakeHitLoc = VictimComp->GetComponentLocation();
						FVector const FakeHitNorm = (GetActorLocation() - FakeHitLoc).GetSafeNormal();
						OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp.Get(), FakeHitLoc, FakeHitNorm);
						
						AActor* ExplosiveHitActor = VictimComp.Get()->GetOwner();

						if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
						{
							if (AMultiplayerCharacter* EnemyCast = Cast<AMultiplayerCharacter>(ExplosiveHitActor))
							{
								if (EnemyCast->GetHealthComponent())
								{
									if (EnemyCast->GetHealthComponent()->GetHealth() > 0)
									{
										if (UseActorClassesForHitMarkers == 0 && ExplosiveHitActor)
										{
											PlayerCast->ShowHitMarker(ExplosiveHitActor, nullptr);
											break;
										}
									}
								}
							}
						}
					}
				}
				
				if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
				{
					PlayerCast->ShowHitMarker(nullptr, nullptr);
				}
			}
		}

		if (BulletHitMode != 0 && BulletHitModeDelay <= 0.0f)
		{
			ExecuteHitFunction(OwningPlayer);
		}

		SpawnHitParticleEffect(DefaultHitEffect, HitLocation, Hit.ImpactNormal.Rotation(), Hit, UseCurrentLocationForHit);

		if (DestructionSphereToSpawn && CanCrumbleDestructibleMeshes == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			GetWorld()->SpawnActor<AActor>(DestructionSphereToSpawn, FTransform(HitLocation.Rotation(), HitLocation + (Hit.ImpactNormal * -3.0f), DestructionSphereSize), SpawnParams);
		}
	}

	RegisteredHit = true;

	MulticastSilenceBulletWhizzingSound();

	if (BulletHitMode == 0 || BulletHitModeDelay <= 0.0f)
	{
		DestroySelf();
	}

	MulticastSoftDestroyProjectile();

	ProjectileMesh->OnComponentHit.RemoveDynamic(this, &AMultiplayerProjectile::OnProjectileMeshHit);
}

void AMultiplayerProjectile::DetermineBeginPlayDespawnTimer()
{
	if (TimeToDespawnProjectile > 0.0f)
	{
		if (IsExplosive == true)
		{
			BeginPlayExplosionTimerDelegate.BindUFunction(this, "Explode", nullptr, true);
			GetWorldTimerManager().SetTimer(BeginPlayDespawnTimerHandle, BeginPlayExplosionTimerDelegate, TimeToDespawnProjectile, false, TimeToDespawnProjectile);
		}
		else
		{
			GetWorldTimerManager().SetTimer(BeginPlayDespawnTimerHandle, this, &AMultiplayerProjectile::DestroySelf, TimeToDespawnProjectile, false, TimeToDespawnProjectile);
		}
	}
}

void AMultiplayerProjectile::DetermineBulletHitModeDelay()
{
	if (BulletHitModeDelay > 0.0f)
	{
		BulletHitModeTimerDelegate.BindUFunction(this, "ExecuteHitFunction", OwningPlayer, nullptr);
		GetWorldTimerManager().SetTimer(BulletHitModeTimerHandle, BulletHitModeTimerDelegate, BulletHitModeDelay, false, BulletHitModeDelay);
	}
}

void AMultiplayerProjectile::ExecuteHitFunction_Implementation(AActor* ParentPlayer, AActor* HitActor)
{
	if (RegisteredHit == true)
	{
		DestroySelf();
	}
}

void AMultiplayerProjectile::SoftDestroyProjectile()
{
	if (HasAuthority())
	{
		MulticastSoftDestroyProjectile();
	}
	else
	{
		ServerSoftDestroyProjectile();
	}
}

void AMultiplayerProjectile::ServerSoftDestroyProjectile_Implementation()
{
	MulticastSoftDestroyProjectile();
}

void AMultiplayerProjectile::MulticastSoftDestroyProjectile_Implementation()
{
	if (ProjectileMesh)
	{
		ProjectileMesh->SetSimulatePhysics(false);
		ProjectileMesh->SetHiddenInGame(true);
		ProjectileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ProjectileMovement->Deactivate();
	}
}

void AMultiplayerProjectile::SpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit)
{
	if (HasAuthority())
	{
		if (UseCurrentLocationForHit == true)
		{
			FHitResult HitResult;
			MulticastSpawnHitParticleEffect(ParticleEffect, ImpactPoint, FRotator::ZeroRotator, HitResult);
		}
		else
		{
			MulticastSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
		}
	}
	else
	{
		if (UseCurrentLocationForHit == true)
		{
			FHitResult HitResult;
			ServerSpawnHitParticleEffect(ParticleEffect, ImpactPoint, FRotator::ZeroRotator, HitResult);
		}
		else
		{
			ServerSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
		}
	}
}

void AMultiplayerProjectile::ServerSpawnHitParticleEffect_Implementation(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit)
{
	MulticastSpawnHitParticleEffect(ParticleEffect, ImpactPoint, ImpactRotation, Hit);
}

void AMultiplayerProjectile::MulticastSpawnHitParticleEffect_Implementation(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit)
{
	if (ParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleEffect, ImpactPoint, ImpactRotation, HitEffectScale);
	}

	USoundBase* ChosenHitSound;

	TArray<USoundBase*> HitSounds;
	TArray<UPhysicalMaterial*> HitPhysicalMaterials;

	BulletHitSounds.GenerateKeyArray(HitSounds);
	BulletHitSounds.GenerateValueArray(HitPhysicalMaterials);

	UPhysicalMaterial* HitPhysicalMaterial = Hit.PhysMaterial.Get();

	if (HitPhysicalMaterials.Contains(HitPhysicalMaterial))
	{
		int HitSoundIndex;
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

	if (BulletHitControllerVibration)
	{
		UGameplayStatics::SpawnForceFeedbackAtLocation(GetWorld(), BulletHitControllerVibration, ImpactPoint, FRotator::ZeroRotator, false, 1.0f, 0.0f, BulletHitControllerVibrationAttenuation);
	}

	UMaterialInterface* ChosenDecal;

	TArray<UMaterialInterface*> DecalMaterials;
	TArray<UPhysicalMaterial*> DecalPhysicalMaterials;

	BulletHitDecals.GenerateKeyArray(DecalMaterials);
	BulletHitDecals.GenerateValueArray(DecalPhysicalMaterials);

	if (DecalPhysicalMaterials.Contains(HitPhysicalMaterial))
	{
		int DecalIndex;
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

void AMultiplayerProjectile::SilenceBulletWhizzingSound()
{
	if (HasAuthority())
	{
		MulticastSilenceBulletWhizzingSound();
	}
	else
	{
		ServerSilenceBulletWhizzingSound();
	}
}

void AMultiplayerProjectile::ServerSilenceBulletWhizzingSound_Implementation()
{
	MulticastSilenceBulletWhizzingSound();
}

void AMultiplayerProjectile::MulticastSilenceBulletWhizzingSound_Implementation()
{
	if (BulletWhizzingSoundComponent)
	{
		BulletWhizzingSoundComponent->SetVolumeMultiplier(0.0f);
		BulletWhizzingSoundComponent->Deactivate();
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

void AMultiplayerProjectile::SetLaunchPhysicsObjects(bool NewLaunchPhysicsObjects)
{
	LaunchPhysicsObjects = NewLaunchPhysicsObjects;
}

bool AMultiplayerProjectile::GetLaunchPhysicsObjects()
{
	return LaunchPhysicsObjects;
}

void AMultiplayerProjectile::SetLaunchObjectStrength(float NewLaunchObjectStrength)
{
	LaunchObjectStrength = NewLaunchObjectStrength;

	if (RadialForceComponent)
	{
		RadialForceComponent->ImpulseStrength = NewLaunchObjectStrength;
	}
}

float AMultiplayerProjectile::GetLaunchObjectStrength()
{
	return LaunchObjectStrength;
}

void AMultiplayerProjectile::SetLaunchObjectVelocityChange(bool NewLaunchObjectVelocityChange)
{
	LaunchObjectVelocityChange = NewLaunchObjectVelocityChange;

	if (RadialForceComponent)
	{
		RadialForceComponent->bImpulseVelChange = NewLaunchObjectVelocityChange;
	}
}

bool AMultiplayerProjectile::GetLaunchObjectVelocityChange()
{
	return LaunchObjectVelocityChange;
}

void AMultiplayerProjectile::SetIsExplosive(bool NewIsExplosive)
{
	IsExplosive = NewIsExplosive;
}

bool AMultiplayerProjectile::GetIsExplosive()
{
	return IsExplosive;
}

void AMultiplayerProjectile::SetHitEffectScale(FVector NewHitEffectScale)
{
	HitEffectScale = NewHitEffectScale;
}

FVector AMultiplayerProjectile::GetHitEffectScale()
{
	return HitEffectScale;
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

void AMultiplayerProjectile::SetTimeToDespawnProjectile(float NewTimeToDespawnProjectile)
{
	TimeToDespawnProjectile = NewTimeToDespawnProjectile;
}

float AMultiplayerProjectile::GetTimeToDespawnProjectile()
{
	return TimeToDespawnProjectile;
}

void AMultiplayerProjectile::SetDefaultDamage(float NewDefaultDamage)
{
	DefaultDamage = NewDefaultDamage;
}

float AMultiplayerProjectile::GetDefaultDamage()
{
	return DefaultDamage;
}

void AMultiplayerProjectile::SetDamage(TMap<UPhysicalMaterial*, float> NewDamage)
{
	Damage = NewDamage;
}

TMap<UPhysicalMaterial*, float> AMultiplayerProjectile::GetDamage()
{
	return Damage;
}

void AMultiplayerProjectile::SetExplosionIgnoreOwner(bool NewExplosionIgnoreOwner)
{
	ExplosionIgnoreOwner = NewExplosionIgnoreOwner;
}

bool AMultiplayerProjectile::GetExplosionIgnoreOwner()
{
	return ExplosionIgnoreOwner;
}

void AMultiplayerProjectile::SetBulletHitMode(int NewBulletHitMode)
{
	BulletHitMode = NewBulletHitMode;
}

int AMultiplayerProjectile::GetBulletHitMode()
{
	return BulletHitMode;
}

void AMultiplayerProjectile::SetBulletHitModeDelay(float NewBulletHitModeDelay)
{
	BulletHitModeDelay = NewBulletHitModeDelay;
}

float AMultiplayerProjectile::GetBulletHitModeDelay()
{
	return BulletHitModeDelay;
}

void AMultiplayerProjectile::SetExplosiveDamageRadius(float NewExplosiveDamageRadius)
{
	ExplosiveDamageRadius = NewExplosiveDamageRadius;

	if (RadialForceComponent)
	{
		RadialForceComponent->Radius = NewExplosiveDamageRadius;
	}
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

void AMultiplayerProjectile::SetExplosiveCollisionChannel(TEnumAsByte<ECollisionChannel> NewExplosiveCollisionChannel)
{
	ExplosiveCollisionChannel = NewExplosiveCollisionChannel;
}

TEnumAsByte<ECollisionChannel> AMultiplayerProjectile::GetExplosiveCollisionChannel()
{
	return ExplosiveCollisionChannel;
}

void AMultiplayerProjectile::SetHitDirection(FVector NewHitDirection)
{
	HitDirection = NewHitDirection;
}

FVector AMultiplayerProjectile::GetHitDirection()
{
	return HitDirection;
}

void AMultiplayerProjectile::SetDefaultHitEffect(UParticleSystem* NewDefaultHitEffect)
{
	DefaultHitEffect = NewDefaultHitEffect;
}

UParticleSystem* AMultiplayerProjectile::GetDefaultHitEffect()
{
	return DefaultHitEffect;
}

void AMultiplayerProjectile::SetHitEffects(TMap<UPhysicalMaterial*, UParticleSystem*> NewHitEffects)
{
	HitEffects = NewHitEffects;
}

TMap<UPhysicalMaterial*, UParticleSystem*> AMultiplayerProjectile::GetHitEffects()
{
	return HitEffects;
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

void AMultiplayerProjectile::SetUseActorClassesForHitMarkers(int NewUseActorClassesForHitMarkers)
{
	UseActorClassesForHitMarkers = NewUseActorClassesForHitMarkers;
}

int AMultiplayerProjectile::GetUseActorClassesForHitMarkers()
{
	return UseActorClassesForHitMarkers;
}

void AMultiplayerProjectile::SetHitMarkerActorSounds(TMap<TSubclassOf<AActor>, USoundBase*> NewHitMarkerActorSounds)
{
	HitMarkerActorSounds = NewHitMarkerActorSounds;
}

TMap<TSubclassOf<AActor>, USoundBase*> AMultiplayerProjectile::GetHitMarkerActorSounds()
{
	return HitMarkerActorSounds;
}

void AMultiplayerProjectile::SetHitMarkerSurfaceSounds(TMap<UPhysicalMaterial*, USoundBase*> NewHitMarkerSurfaceSounds)
{
	HitMarkerSurfaceSounds = NewHitMarkerSurfaceSounds;
}

TMap<UPhysicalMaterial*, USoundBase*> AMultiplayerProjectile::GetHitMarkerSurfaceSounds()
{
	return HitMarkerSurfaceSounds;
}

void AMultiplayerProjectile::SetCanCrumbleDestructibleMeshes(bool NewCanCrumbleDestructibleMeshes)
{
	CanCrumbleDestructibleMeshes = NewCanCrumbleDestructibleMeshes;
}

bool AMultiplayerProjectile::GetCanCrumbleDestructibleMeshes()
{
	return CanCrumbleDestructibleMeshes;
}

void AMultiplayerProjectile::SetDestructionSphereSize(FVector NewDestructionSphereSize)
{
	DestructionSphereSize = NewDestructionSphereSize;
}

FVector AMultiplayerProjectile::GetDestructionSphereSize()
{
	return DestructionSphereSize;
}

void AMultiplayerProjectile::SetDestructionSphereToSpawn(TSubclassOf<AActor> NewDestructionSphereToSpawn)
{
	DestructionSphereToSpawn = NewDestructionSphereToSpawn;
}

TSubclassOf<AActor> AMultiplayerProjectile::GetDestructionSphereToSpawn()
{
	return DestructionSphereToSpawn;
}

void AMultiplayerProjectile::SetBulletHitControllerVibration(UForceFeedbackEffect* NewBulletHitControllerVibration)
{
	BulletHitControllerVibration = NewBulletHitControllerVibration;
}

UForceFeedbackEffect* AMultiplayerProjectile::GetBulletHitControllerVibration()
{
	return BulletHitControllerVibration;
}

void AMultiplayerProjectile::SetBulletHitControllerVibrationAttenuation(UForceFeedbackAttenuation* NewBulletHitControllerVibrationAttenuation)
{
	BulletHitControllerVibrationAttenuation = NewBulletHitControllerVibrationAttenuation;
}

UForceFeedbackAttenuation* AMultiplayerProjectile::GetBulletHitControllerVibrationAttenuation()
{
	return BulletHitControllerVibrationAttenuation;
}

void AMultiplayerProjectile::SetBulletHitControllerVibrationTag(FName NewBulletHitControllerVibrationTag)
{
	BulletHitControllerVibrationTag = NewBulletHitControllerVibrationTag;
}

FName AMultiplayerProjectile::GetBulletHitControllerVibrationTag()
{
	return BulletHitControllerVibrationTag;
}

// Called when the game starts or when spawned
void AMultiplayerProjectile::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerProjectile::DetermineBulletHitModeDelay);
	GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerProjectile::DetermineBeginPlayDespawnTimer);
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
	DOREPLIFETIME(AMultiplayerProjectile, HitEffectScale);
	DOREPLIFETIME(AMultiplayerProjectile, RegisteredHit);
	DOREPLIFETIME(AMultiplayerProjectile, DefaultBulletHitSound);
}