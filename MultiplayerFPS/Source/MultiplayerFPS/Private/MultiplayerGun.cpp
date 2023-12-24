// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerGun.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerFPS.h"
#include "Camera/CameraShakeBase.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMultiplayerGun::AMultiplayerGun()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set this to false to use a static mesh and close the engine to recompile (hot reload will not work)
	UseSkeletalMesh = true;

	if (UseSkeletalMesh == true)
	{
		GunSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Skeletal Mesh"));
		RootComponent = GunSkeletalMesh;
		ThirdPersonGunSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Gun Skeletal Mesh"));
		ThirdPersonGunSkeletalMesh->SetupAttachment(RootComponent, NAME_None);
		GunSkeletalMesh->SetOnlyOwnerSee(true);
		GunSkeletalMesh->SetCollisionProfileName("NoCollision");
		GunSkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		ThirdPersonGunSkeletalMesh->SetOwnerNoSee(true);
		ThirdPersonGunSkeletalMesh->SetCollisionProfileName("NoCollision");
	}
	else
	{
		GunStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Static Mesh"));
		RootComponent = GunStaticMesh;
		ThirdPersonGunStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Third Person Gun Static Mesh"));
		ThirdPersonGunStaticMesh->SetupAttachment(RootComponent, NAME_None);
		GunStaticMesh->SetOnlyOwnerSee(true);
		GunStaticMesh->SetCollisionProfileName("NoCollision");
		GunStaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		ThirdPersonGunStaticMesh->SetOwnerNoSee(true);
		ThirdPersonGunStaticMesh->SetCollisionProfileName("NoCollision");
	}

	FireSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fire Scene Component"));
	FireSceneComponent->SetupAttachment(RootComponent, NAME_None);
	BulletCasingSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Casing Scene Component"));
	BulletCasingSceneComponent->SetupAttachment(RootComponent, NAME_None);
	PickupBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Pickup Box Collision"));
	PickupBoxCollision->SetupAttachment(RootComponent, NAME_None);

	PickupBoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMultiplayerGun::OnPickupBoxCollisionBeginOverlap);
	PickupBoxCollision->OnComponentEndOverlap.AddDynamic(this, &AMultiplayerGun::OnPickupBoxCollisionEndOverlap);

	CanShoot = true;
	FireMode = 0;
	IsShotgun = false;
	ShotgunAmountOfPellets = 8;
	ShotgunAmountOfPelletsShot = 0;
	ShotgunMinVerticalSpread = -1.75f;
	ShotgunMaxVerticalSpread = 1.75f;
	ShotgunMinHorizontalSpread = -1.75f;
	ShotgunMaxHorizontalSpread = 1.75f;
	HaveBulletSpread = 0;
	SpreadAimingMultiplier = 0.65f;
	MinHipFireVerticalSpread = 0.0f;
	MaxHipFireVerticalSpread = 0.0f;
	MinHipFireHorizontalSpread = 0.0f;
	MaxHipFireHorizontalSpread = 0.0f;
	AmmoInMagazine = 30;
	MaxAmmoInMagazine = 30;
	ReserveAmmo = 120;
	MaxReserveAmmo = 120;
	DoesOverheat = false;
	TimeToOverheat = 5.0f;
	MaxHeat = 50.0f;
	CurrentHeat = 0.0f;
	IsOverheating = false;
	BulletCasingSpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	BulletCasingInheritsVelocity = true;
	MaxAmountOfBulletCasings = 30;
	ReloadSpeed = 0.5f;
	ReloadSpeed1 = 0.4f;
	FireRate = 0.1f;
	ContinuousFireDamageRate = 0.005f;
	AmountOfShotsForBurst = 3;
	AmountOfBurstShotsFired = 0;
	FireRange = 100000000.0f;
	IsExplosive = false;
	ExplosionScale = FVector(1.0f, 1.0f, 1.0f);
	ExplosionIgnoreOwner = false;
	UseProjectile = false;
	ProjectileSpawnForwardOffset = 25.0f;
	ProjectileSpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ProjectileInheritsVelocity = false;
	DefaultDamage = 35.0f;
	HeadDamage = 75.0f;
	TorsoDamage = 30.0f;
	LegDamage = 20.0f;
	ExplosiveDamageRadius = 250.0f;
	ExplosiveDoFullDamage = false;
	WasPickedup = false;
	SnapToSocket = 0;
	BulletWhizzingSoundVolumeBasedOnSpeed = true;
	UseADS = 0;
	ADSFOV = 15.0f;
	ZoomFOV = 10.0f;
	TimeToADS = 0.15f;
	TimeToZoom = 0.15f;
	VerticalRecoil = 1.0f;
	HorizontalRecoil = 0.25f;
	ShowBulletPath = false;
	BulletPathColor = FColor::Red;
	BulletPathPersists = false;
	BulletPathDuration = 10.0f;
	PlayerArmsRelativeLocation = FVector(-6.0f, -1.0f, -154.0f);
	PlayerArmsRelativeRotation = FRotator(0.0f, -5.0f, 0.0f);
	SocketName = "GripPoint";
}

void AMultiplayerGun::CheckForOwner()
{
	if (WasPickedup == true)
	{
		bool DestroySelf;

		if (OwningPlayer)
		{
			if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
			{
				if (PlayerCast->GetHealthComponent()->GetHealth() >= 1)
				{
					DestroySelf = false;
				}
				else
				{
					DestroySelf = true;
				}
			}
			else
			{
				DestroySelf = true;
			}
		}
		else
		{
			DestroySelf = true;
		}

		if (DestroySelf == true)
		{
			GetWorldTimerManager().ClearTimer(CheckForOwnerTimerHandle);

			Destroy();
		}
	}
}

void AMultiplayerGun::OnPickupBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (WasPickedup == false && OtherActor)
	{
		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OtherActor))
		{
			PlayerCast->SetOverlappingWeapon(true);
			PlayerCast->SetWeaponBeingOverlapped(this);
		}
	}
}

void AMultiplayerGun::OnPickupBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OtherActor))
		{
			PlayerCast->SetOverlappingWeapon(false);
			PlayerCast->SetWeaponBeingOverlapped(nullptr);
		}
	}
}

void AMultiplayerGun::SetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo)
{
	if (HasAuthority())
	{
		MulticastSetWasPickedup(Pickedup, ComponentToAttachTo);
	}
	else
	{
		ServerSetWasPickedup(Pickedup, ComponentToAttachTo);
	}
}

void AMultiplayerGun::ServerSetWasPickedup_Implementation(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo)
{
	MulticastSetWasPickedup(Pickedup, ComponentToAttachTo);
}

void AMultiplayerGun::MulticastSetWasPickedup_Implementation(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo)
{
	WasPickedup = Pickedup;
	UPrimitiveComponent* GunMeshComponent;
	UPrimitiveComponent* ThirdPersonGunMeshComponent;

	if (UseSkeletalMesh == true)
	{
		GunMeshComponent = GunSkeletalMesh;
		ThirdPersonGunMeshComponent = ThirdPersonGunSkeletalMesh;
	}
	else
	{
		GunMeshComponent = GunStaticMesh;
		ThirdPersonGunMeshComponent = ThirdPersonGunStaticMesh;
	}

	if (Pickedup == true)
	{
		PickupBoxCollision->SetGenerateOverlapEvents(false);

		if (ComponentToAttachTo)
		{
			GunMeshComponent->SetSimulatePhysics(false);
			GunMeshComponent->SetCollisionProfileName("NoCollision");
			GunMeshComponent->SetOnlyOwnerSee(true);
			ThirdPersonGunMeshComponent->SetHiddenInGame(false);
			ThirdPersonGunMeshComponent->SetVisibility(true);

			if (SnapToSocket == 0)
			{
				AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::KeepWorldTransform, SocketName);

				SetActorRelativeLocation(GunRelativeLocation);
				SetActorRelativeRotation(GunRelativeRotation);
			}
			else if (SnapToSocket == 1)
			{
				AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			}
			else
			{
				AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			}

			if (OwningPlayer)
			{
				if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
				{
					PlayerCast->SetOverlappingWeapon(false);
					PlayerCast->SetWeaponBeingOverlapped(nullptr);
				}
			}
		}
	}
	else
	{
		StopFiring();

		ThirdPersonGunMeshComponent->SetHiddenInGame(true);
		ThirdPersonGunMeshComponent->SetVisibility(false);

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		GunMeshComponent->SetCollisionProfileName("BlockAllDynamic");
		GunMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GunMeshComponent->SetSimulatePhysics(true);
		GunMeshComponent->SetOnlyOwnerSee(false);

		SetActorHiddenInGame(false);

		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerGun::SetPickupCollisionEnabled);
	}
}

void AMultiplayerGun::SetPickupCollisionEnabled()
{
	if (HasAuthority())
	{
		MulticastSetPickupCollisionEnabled();
	}
	else
	{
		ServerSetPickupCollisionEnabled();
	}
}

void AMultiplayerGun::ServerSetPickupCollisionEnabled_Implementation()
{
	MulticastSetPickupCollisionEnabled();
}

void AMultiplayerGun::MulticastSetPickupCollisionEnabled_Implementation()
{
	PickupBoxCollision->SetGenerateOverlapEvents(true);
}

APawn* AMultiplayerGun::GetOwningPlayer()
{
	return OwningPlayer;
}

void AMultiplayerGun::SetOwningPlayer(APawn* NewOwningPlayer)
{
	OwningPlayer = NewOwningPlayer;
}

void AMultiplayerGun::FireInput()
{
	if (CanShoot == true && AmmoInMagazine >= 1 && OwningPlayer && WasPickedup == true && CurrentHeat < MaxHeat)
	{
		if (IsShotgun == false || IsShotgun == true && ShotgunAmountOfPelletsShot == 0)
		{
			if (FireMode == 0)
			{
				if (IsShotgun == true)
				{
					ShotgunFire();
				}
				else
				{
					Fire();
				}
			}
			else if (FireMode == 1)
			{
				if (IsShotgun == true)
				{
					ShotgunFire();

					GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::ShotgunFire, FireRate, true, FireRate);
				}
				else
				{
					Fire();

					GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::Fire, FireRate, true, FireRate);
				}
			}
			else if (FireMode == 2 && AmountOfBurstShotsFired <= 0)
			{
				if (IsShotgun == true)
				{
					ShotgunFire();

					GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &AMultiplayerGun::ShotgunFire, FireRate, true, FireRate);
				}
				else
				{
					Fire();

					GetWorldTimerManager().SetTimer(BurstFireTimerHandle, this, &AMultiplayerGun::Fire, FireRate, true, FireRate);
				}
			}
			else if (FireMode == 3)
			{
				ContinuousFire();

				GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::ContinuousFire, ContinuousFireDamageRate, true, ContinuousFireDamageRate);
			}
		}
	}
	else if (FireMode == 1 && AmmoInMagazine >= 1 && CurrentHeat < MaxHeat - 1)
	{
		if (IsShotgun == true)
		{
			GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::ShotgunFire, FireRate, true, FireRate);
		}
		else
		{
			GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::Fire, FireRate, true, FireRate);
		}
	}
}

void AMultiplayerGun::Fire()
{
	if (!HasAuthority() && UseProjectile == false)
	{
		ServerFire();
	}

	if (IsShotgun == false)
	{
		if (FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsForBurst || FireMode == 2 && AmmoInMagazine <= 0)
		{
			GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);
			AmountOfBurstShotsFired = 0;
			return;
		}
		else if (FireMode == 2)
		{
			AmountOfBurstShotsFired++;
		}
	}

	if (AmmoInMagazine > 0 && CurrentHeat < MaxHeat || IsShotgun == true && ShotgunAmountOfPelletsShot > 0)
	{
		if (OwningPlayer && WasPickedup == true)
		{
			if (CanShoot == true || FireMode == 1 || FireMode == 2 || IsShotgun == true)
			{
				FVector FireLocation;
				FRotator FireRotation;
				OwningPlayer->GetActorEyesViewPoint(FireLocation, FireRotation);

				FVector TraceDirection;

				if (IsShotgun == true)
				{
					FireRotation += FRotator(FMath::RandRange(ShotgunMinVerticalSpread, ShotgunMaxVerticalSpread), FMath::RandRange(ShotgunMinHorizontalSpread, ShotgunMaxHorizontalSpread), 0.0f);
				}
				else if (HaveBulletSpread != 0)
				{
					if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
					{
						if (HaveBulletSpread == 1 && PlayerCast->GetIsAiming() == false || HaveBulletSpread == 2 && PlayerCast->GetIsADSing() == false || HaveBulletSpread == 3 && PlayerCast->GetIsZoomedIn() == false || HaveBulletSpread == 4)
						{
							if (PlayerCast->GetIsAiming())
							{
								FireRotation += FRotator(FMath::RandRange(MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), FMath::RandRange(MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0f);
							}
							else
							{
								FireRotation += FRotator(FMath::RandRange(MinHipFireVerticalSpread, MaxHipFireVerticalSpread), FMath::RandRange(MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0f);
							}
						}
					}
				}

				TraceDirection = FireRotation.Vector();

				GunHitEffectsReplication.FireLocation = FireLocation;

				if (UseProjectile == false)
				{
					FVector TraceEnd = FireLocation + (TraceDirection * FireRange);

					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(this);
					QueryParams.AddIgnoredActor(OwningPlayer);
					QueryParams.bTraceComplex = true;
					QueryParams.bReturnPhysicalMaterial = true;

					FHitResult Hit;
					if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, TraceEnd, ECC_Visibility, QueryParams))
					{
						AActor* HitActor = Hit.GetActor();
						EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
						UParticleSystem* HitEffect;

						if (IsExplosive == true)
						{
							HitEffect = DefaultHitEffect;
						}
						else
						{
							if (SurfaceType == SURFACE_HEAD || SurfaceType == SURFACE_CHEST || SurfaceType == SURFACE_TORSO || SurfaceType == SURFACE_LEG)
							{
								HitEffect = Blood;
							}
							else
							{
								HitEffect = DefaultHitEffect;
							}
						}

						GunHitEffectsReplication.HitLocation = Hit.ImpactPoint;
						GunHitEffectsReplication.HitRotation = Hit.ImpactNormal.Rotation();
						GunHitEffectsReplication.HitEffect = HitEffect;
						GunHitEffectsReplication.HitResult = Hit;

						if (HitEffect)
						{
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, Hit.ImpactPoint);
						}

						USoundBase* ChosenHitSound;

						if (IsExplosive == false)
						{
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
						}
						else
						{
							ChosenHitSound = DefaultBulletHitSound;
						}

						if (ChosenHitSound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChosenHitSound, Hit.ImpactPoint);
						}

						if (IsExplosive == true && DefaultDamage > 0 && ExplosiveDamageRadius > 0)
						{
							FVector HitLocation = Hit.ImpactPoint - (TraceDirection * 0.1f);

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

							UGameplayStatics::ApplyRadialDamage(GetWorld(), DefaultDamage, HitLocation, ExplosiveDamageRadius, DamageType, IgnoredActors, this, GetInstigatorController(), ExplosiveDoFullDamage, ECC_Visibility);
						}
						else
						{
							UMaterialInterface* ChosenDecal;

							int DecalIndex;
							TArray<UMaterialInterface*> DecalMaterials;
							TArray<UPhysicalMaterial*> DecalPhysicalMaterials;

							BulletHitDecals.GenerateKeyArray(DecalMaterials);
							BulletHitDecals.GenerateValueArray(DecalPhysicalMaterials);

							UPhysicalMaterial* HitPhysicalMaterial = Hit.PhysMaterial.Get();

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

							if (SurfaceType == SURFACE_HEAD)
							{
								UGameplayStatics::ApplyPointDamage(HitActor, HeadDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
							}
							else if (SurfaceType == SURFACE_TORSO)
							{
								UGameplayStatics::ApplyPointDamage(HitActor, TorsoDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
							}
							else if (SurfaceType == SURFACE_LEG)
							{
								UGameplayStatics::ApplyPointDamage(HitActor, LegDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
							}
							else
							{
								UGameplayStatics::ApplyPointDamage(HitActor, DefaultDamage, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
							}
						}
					}

					/* ************* Debug ************* */

					if (ShowBulletPath == true)
					{
						DrawDebugLine(GetWorld(), FireLocation, TraceEnd, BulletPathColor, BulletPathPersists, BulletPathDuration);
					}

					/* ************* Debug ************* */
				}
				else
				{
					SpawnProjectile(FireLocation, FireRotation, TraceDirection);
				}

				if (IsShotgun == false)
				{
					if (FireMode != 3)
					{
						if (FireSound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, FireSceneComponent->GetComponentLocation());
						}

						if (MuzzleFlash)
						{
							UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, FireSceneComponent);
						}
					}

					if (FireArmsAnimationMontage)
					{
						if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
						{
							if (PlayerCast->ArmsMesh && PlayerCast->GetIsADSing() == false)
							{
								if (UAnimInstance* AnimationInstance = PlayerCast->ArmsMesh->GetAnimInstance())
								{
									AnimationInstance->Montage_Play(FireArmsAnimationMontage);
								}
							}
						}
					}

					if (FireCameraShake && OwningPlayer)
					{
						if (APlayerController* OwningController = Cast<APlayerController>(OwningPlayer->GetController()))
						{
							OwningController->ClientStartCameraShake(FireCameraShake);
						}
					}

					AddRecoil_BP();

					if (HasAuthority())
					{
						SpawnBulletCasing();
					}

					if (FireMode != 3)
					{
						CanShoot = false;
					}

					if (DoesOverheat == true)
					{
						if (IsOverheating == false)
						{
							IsOverheating = true;

							Overheat_BP();
						}
					}
					else
					{
						AmmoInMagazine--;
					}
				}

				if (IsShotgun == false && DoesOverheat == false)
				{
					if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
					{
						if (AmmoInMagazine <= 0)
						{
							PlayerCast->Reload();
						}
					}
				}

				if (IsShotgun == false)
				{
					FireTimerDelegate.BindUFunction(this, FName("SetCanShoot"), true);
					GetWorldTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, FireRate, false, FireRate);
				}
			}
		}
	}
	else if (CurrentHeat >= MaxHeat)
	{
		StopFiring();
	}
}

void AMultiplayerGun::ServerFire_Implementation()
{
	Fire();
}

void AMultiplayerGun::ShotgunFire()
{
	if (!HasAuthority())
	{
		ServerShotgunFire();
	}

	if (FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsForBurst || FireMode == 2 && AmmoInMagazine <= 0)
	{
		GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);
		AmountOfBurstShotsFired = 0;
		return;
	}
	else if (FireMode == 2)
	{
		AmountOfBurstShotsFired++;
	}

	for (int32 Index = 0; Index != ShotgunAmountOfPellets; ++Index)
	{
		Fire();
		ShotgunAmountOfPelletsShot++;
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, FireSceneComponent->GetComponentLocation());
	}

	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, FireSceneComponent);
	}

	if (FireArmsAnimationMontage)
	{
		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
		{
			if (PlayerCast->ArmsMesh && PlayerCast->GetIsADSing() == false)
			{
				if (UAnimInstance* AnimationInstance = PlayerCast->ArmsMesh->GetAnimInstance())
				{
					AnimationInstance->Montage_Play(FireArmsAnimationMontage);
				}
			}
		}
	}

	if (FireCameraShake && OwningPlayer)
	{
		if (APlayerController* OwningController = Cast<APlayerController>(OwningPlayer->GetController()))
		{
			OwningController->ClientStartCameraShake(FireCameraShake);
		}
	}

	AddRecoil_BP();

	if (HasAuthority())
	{
		SpawnBulletCasing();
	}

	ShotgunAmountOfPelletsShot = 0;

	CanShoot = false;

	AmmoInMagazine--;

	if (OwningPlayer && AmmoInMagazine <= 0)
	{
		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
		{
			PlayerCast->Reload();
		}
	}

	FireTimerDelegate.BindUFunction(this, FName("SetCanShoot"), true);
	GetWorldTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, FireRate, false, FireRate);
}

void AMultiplayerGun::ServerShotgunFire_Implementation()
{
	ShotgunFire();
}

void AMultiplayerGun::ContinuousFire()
{
	if (CurrentHeat < MaxHeat)
	{
		if (!HasAuthority())
		{
			ServerContinuousFire();
		}

		if (IsShotgun == true)
		{
			ShotgunFire();
		}
		else
		{
			Fire();
		}

		if (DoesOverheat == true && IsOverheating == false)
		{
			IsOverheating = true;

			Overheat_BP();
		}

		if (FireSound && !SpawnedContinuousFireSound)
		{
			SpawnedContinuousFireSound = UGameplayStatics::SpawnSoundAttached(FireSound, FireSceneComponent);
		}

		if (MuzzleFlash && !SpawnedContinuousMuzzleFlash)
		{
			SpawnedContinuousMuzzleFlash = UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, FireSceneComponent);
		}

		FireTimerDelegate.BindUFunction(this, FName("SetCanShoot"), true);
		GetWorldTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, FireRate, false, FireRate);
	}
	else
	{
		StopFiring();
	}
}

void AMultiplayerGun::ServerContinuousFire_Implementation()
{
	ContinuousFire();
}

void AMultiplayerGun::OnRep_GunHitEffects()
{
	if (GunHitEffectsReplication.HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunHitEffectsReplication.HitEffect, GunHitEffectsReplication.HitLocation);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GunHitEffectsReplication.FireLocation);
	}

	USoundBase* ChosenHitSound;

	int HitSoundIndex;
	TArray<USoundBase*> HitSounds;
	TArray<UPhysicalMaterial*> HitPhysicalMaterials;

	BulletHitSounds.GenerateKeyArray(HitSounds);
	BulletHitSounds.GenerateValueArray(HitPhysicalMaterials);

	UPhysicalMaterial* HitPhysicalMaterial = GunHitEffectsReplication.HitResult.PhysMaterial.Get();

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
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChosenHitSound, GunHitEffectsReplication.HitLocation);
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
		UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ChosenDecal, BulletHitDecalSize, GunHitEffectsReplication.HitLocation, GunHitEffectsReplication.HitRotation);
	}
}

void AMultiplayerGun::SpawnProjectile_Implementation(FVector FireLocation, FRotator FireRotation, FVector TraceDirection)
{
	if (ProjectileToSpawn)
	{
		if (ProjectileSpawnForwardOffset != 0)
		{
			if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
			{
				FireLocation += PlayerCast->FPCameraComponent->GetForwardVector() * ProjectileSpawnForwardOffset;
			}
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ProjectileSpawnCollisionHandlingMethod;

		if (AMultiplayerProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AMultiplayerProjectile>(ProjectileToSpawn, FireLocation, FireRotation, SpawnParams))
		{
			SpawnedProjectile->ProjectileMesh->IgnoreActorWhenMoving(OwningPlayer, true);
			SpawnedProjectile->SetOwningPlayer(OwningPlayer);
			SpawnedProjectile->SetIsExplosive(IsExplosive);
			SpawnedProjectile->SetExplosionScale(ExplosionScale);
			SpawnedProjectile->SetExplosionIgnoredActors(ExplosionIgnoredActors);
			SpawnedProjectile->SetExplosionIgnoredClasses(ExplosionIgnoredClasses);
			SpawnedProjectile->SetDefaultDamage(DefaultDamage);
			SpawnedProjectile->SetHeadDamage(HeadDamage);
			SpawnedProjectile->SetTorsoDamage(TorsoDamage);
			SpawnedProjectile->SetLegDamage(LegDamage);
			SpawnedProjectile->SetExplosiveDamageRadius(ExplosiveDamageRadius);
			SpawnedProjectile->SetExplosionIgnoreOwner(ExplosionIgnoreOwner);
			SpawnedProjectile->SetExplosiveDoFullDamage(ExplosiveDoFullDamage);
			SpawnedProjectile->SetHitDirection(TraceDirection);
			SpawnedProjectile->SetBloodEffect(Blood);
			SpawnedProjectile->SetDefaultHitEffect(DefaultHitEffect);
			SpawnedProjectile->SetDefaultBulletHitSound(DefaultBulletHitSound);
			SpawnedProjectile->SetBulletHitSounds(BulletHitSounds);
			SpawnedProjectile->SetDamageType(DamageType);
			SpawnedProjectile->SetDefaultBulletHitDecal(DefaultBulletHitDecal);
			SpawnedProjectile->SetBulletHitDecals(BulletHitDecals);
			SpawnedProjectile->SetBulletHitDecalSize(BulletHitDecalSize);
			SpawnedProjectile->SetWhizzingSoundVolumeBasedOnSpeed(BulletWhizzingSoundVolumeBasedOnSpeed);

			if (ProjectileInheritsVelocity == true)
			{
				if (ACharacter* CharacterCast = Cast<ACharacter>(OwningPlayer))
				{
					SpawnedProjectile->ProjectileMovement->Velocity += CharacterCast->GetCharacterMovement()->Velocity;
				}
			}

			if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(OwningPlayer))
			{
				for (auto& Weapon : PlayerCast->GetAllWeapons())
				{
					if (Weapon)
					{
						SpawnedProjectile->ProjectileMesh->IgnoreActorWhenMoving(Weapon, true);
					}
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Failed To Spawn Projectile MultiplayerGun.cpp:SpawnProjectile");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "ProjectileToSpawn Invalid MultiplayerGun.cpp:SpawnProjectile");
	}
}

void AMultiplayerGun::SpawnBulletCasing_Implementation()
{
	if (BulletCasingToSpawn)
	{
		CheckBulletCasingLimit();

		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = BulletCasingSpawnCollisionHandlingMethod;

		if (AMultiplayerBulletCasing* SpawnedCasing = GetWorld()->SpawnActor<AMultiplayerBulletCasing>(BulletCasingToSpawn, BulletCasingSceneComponent->GetComponentLocation(), BulletCasingSceneComponent->GetComponentRotation(), SpawnParams))
		{
			SpawnedCasing->SetOwningGun(this);
			SpawnedBulletCasings.EmplaceAt(0, SpawnedCasing);

			if (BulletCasingInheritsVelocity == true && OwningPlayer)
			{
				if (ACharacter* CharacterCast = Cast<ACharacter>(OwningPlayer))
				{
					SpawnedCasing->BulletCasingMesh->AddImpulse(CharacterCast->GetCharacterMovement()->Velocity, NAME_None, true);
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Failed To Spawn Bullet Casing MultiplayerGun.cpp:SpawnBulletCasing");
		}
	}
}

void AMultiplayerGun::CheckBulletCasingLimit_Implementation()
{
	if (SpawnedBulletCasings.Num() >= MaxAmountOfBulletCasings)
	{
		if (AMultiplayerBulletCasing* CasingToDestroy = SpawnedBulletCasings[SpawnedBulletCasings.Num() - 1])
		{
			CasingToDestroy->Destroy();
			SpawnedBulletCasings.Remove(CasingToDestroy);
		}
	}
}

void AMultiplayerGun::StopFiring(bool EvenCancelBurst)
{
	GetWorldTimerManager().ClearTimer(FireFullAutoTimerHandle);

	if (EvenCancelBurst == true)
	{
		GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);

		AmountOfBurstShotsFired = 0;
	}

	if (FireMode == 3)
	{
		if (SpawnedContinuousFireSound)
		{
			SpawnedContinuousFireSound->DestroyComponent();
		}

		if (SpawnedContinuousMuzzleFlash)
		{
			SpawnedContinuousMuzzleFlash->DestroyComponent();
		}
	}

	if (DoesOverheat == true && IsOverheating == true)
	{
		IsOverheating = false;

		CoolDown_BP();
	}
}

void AMultiplayerGun::SetCanShoot(bool NewCanShoot)
{
	CanShoot = NewCanShoot;

	if (NewCanShoot == false)
	{
		StopFiring();
	}
}

bool AMultiplayerGun::GetCanShoot()
{
	return CanShoot;
}

void AMultiplayerGun::SetFireMode(int NewFireMode)
{
	FireMode = NewFireMode;
}

FVector AMultiplayerGun::GetPlayerArmsRelativeLocation()
{
	return PlayerArmsRelativeLocation;
}

FRotator AMultiplayerGun::GetPlayerArmsRelativeRotation()
{
	return PlayerArmsRelativeRotation;
}

int AMultiplayerGun::GetFireMode()
{
	return FireMode;
}

void AMultiplayerGun::SetUseADS(int NewUseADS)
{
	UseADS = NewUseADS;
}

int AMultiplayerGun::GetUseADS()
{
	return UseADS;
}

void AMultiplayerGun::SetADSFOV(float NewADSFOV)
{
	ADSFOV = NewADSFOV;
}

float AMultiplayerGun::GetADSFOV()
{
	return ADSFOV;
}

void AMultiplayerGun::SetZoomFOV(float NewZoomFOV)
{
	ZoomFOV = NewZoomFOV;
}

float AMultiplayerGun::GetZoomFOV()
{
	return ZoomFOV;
}

void AMultiplayerGun::SetTimeToADS(float NewTimeToADS)
{
	TimeToADS = NewTimeToADS;
}

float AMultiplayerGun::GetTimeToADS()
{
	return TimeToADS;
}

void AMultiplayerGun::SetTimeToZoom(float NewTimeToZoom)
{
	TimeToZoom = NewTimeToZoom;
}

float AMultiplayerGun::GetTimeToZoom()
{
	return TimeToZoom;
}

void AMultiplayerGun::SetADSArmsLocation(FVector NewADSArmsLocation)
{
	ADSArmsLocation = NewADSArmsLocation;
}

FVector AMultiplayerGun::GetADSArmsLocation()
{
	return ADSArmsLocation;
}

void AMultiplayerGun::SetADSArmsRotation(FRotator NewADSArmsRotation)
{
	ADSArmsRotation = NewADSArmsRotation;
}

FRotator AMultiplayerGun::GetADSArmsRotation()
{
	return ADSArmsRotation;
}

void AMultiplayerGun::SetZoomArmsLocation(FVector NewZoomArmsLocation)
{
	ZoomArmsLocation = NewZoomArmsLocation;
}

FVector AMultiplayerGun::GetZoomArmsLocation()
{
	return ZoomArmsLocation;
}

void AMultiplayerGun::SetZoomArmsRotation(FRotator NewZoomArmsRotation)
{
	ZoomArmsRotation = NewZoomArmsRotation;
}

FRotator AMultiplayerGun::GetZoomArmsRotation()
{
	return ZoomArmsRotation;
}

void AMultiplayerGun::SetAmmoInMagazine(int NewAmmoInMagazine)
{
	AmmoInMagazine = NewAmmoInMagazine;

	if (AmmoInMagazine <= 0)
	{
		StopFiring();
	}
}

int AMultiplayerGun::GetAmmoInMagazine()
{
	return AmmoInMagazine;
}

int AMultiplayerGun::GetMaxAmmoInMagazine()
{
	return MaxAmmoInMagazine;
}

int AMultiplayerGun::GetReserveAmmo()
{
	return ReserveAmmo;
}

int AMultiplayerGun::GetMaxReserveAmmo()
{
	return MaxReserveAmmo;
}

bool AMultiplayerGun::GetUseProjectile()
{
	return UseProjectile;
}

void AMultiplayerGun::Reload()
{
	if (HasAuthority())
	{
		MulticastReload();
	}
	else
	{
		ServerReload();
	}
}

void AMultiplayerGun::ServerReload_Implementation()
{
	MulticastReload();
}

void AMultiplayerGun::MulticastReload_Implementation()
{
	if (ReserveAmmo >= 1 && AmmoInMagazine < MaxAmmoInMagazine)
	{
		float BulletsShot = MaxAmmoInMagazine - AmmoInMagazine;

		if (BulletsShot <= ReserveAmmo)
		{
			AmmoInMagazine = MaxAmmoInMagazine;
			ReserveAmmo -= BulletsShot;
			BulletsShot = 0;
		}
		else if (BulletsShot > ReserveAmmo)
		{
			AmmoInMagazine += ReserveAmmo;
			ReserveAmmo = 0;
			BulletsShot = 0;
		}
	}
}

// Called when the game starts or when spawned
void AMultiplayerGun::BeginPlay()
{
	Super::BeginPlay();

	if (FireMode == 0 || FireMode == 2)
	{
		DoesOverheat = false;
	}

	GetWorldTimerManager().SetTimer(CheckForOwnerTimerHandle, this, &AMultiplayerGun::CheckForOwner, 0.5f, true, 0.5f);
}

// Called every frame
void AMultiplayerGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMultiplayerGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerGun, OwningPlayer);
	DOREPLIFETIME(AMultiplayerGun, WasPickedup);
	DOREPLIFETIME(AMultiplayerGun, AmmoInMagazine);
	DOREPLIFETIME(AMultiplayerGun, ReserveAmmo);
	DOREPLIFETIME(AMultiplayerGun, ShotgunAmountOfPelletsShot);
	DOREPLIFETIME_CONDITION(AMultiplayerGun, GunHitEffectsReplication, COND_SkipOwner);
}