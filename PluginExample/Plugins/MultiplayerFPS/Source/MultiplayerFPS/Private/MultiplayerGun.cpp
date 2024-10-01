// Fill out your copyright notice in the Description page of Project Settings.
// the reason you'll see weird parenthesis on if statements is for linux compatibility

#include "MultiplayerGun.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerCharacter.h"
#include "MultiplayerPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Algo/Reverse.h"

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
		GunSkeletalMesh->SetIsReplicated(true);
		ThirdPersonGunSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Gun Skeletal Mesh"));
		ThirdPersonGunSkeletalMesh->SetupAttachment(RootComponent, NAME_None);
		ThirdPersonGunSkeletalMesh->SetIsReplicated(true);
		GunSkeletalMesh->SetOnlyOwnerSee(true);
		GunSkeletalMesh->SetCollisionProfileName("NoCollision");
		GunSkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		ThirdPersonGunSkeletalMesh->SetOwnerNoSee(true);
		ThirdPersonGunSkeletalMesh->SetCollisionProfileName("NoCollision");
		ThirdPersonFireSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Third Person Fire Scene Component"));
		ThirdPersonFireSceneComponent->SetupAttachment(ThirdPersonGunSkeletalMesh);
		ThirdPersonFireSceneComponent->SetIsReplicated(true);
		ThirdPersonBulletCasingSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Third Person Bullet Casing Scene Component"));
		ThirdPersonBulletCasingSceneComponent->SetupAttachment(ThirdPersonGunSkeletalMesh);
		ThirdPersonBulletCasingSceneComponent->SetIsReplicated(true);
	}
	else
	{
		GunStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Static Mesh"));
		RootComponent = GunStaticMesh;
		GunStaticMesh->SetIsReplicated(true);
		ThirdPersonGunStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Third Person Gun Static Mesh"));
		ThirdPersonGunStaticMesh->SetupAttachment(RootComponent, NAME_None);
		ThirdPersonFireSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Third Person Fire Scene Component"));
		ThirdPersonFireSceneComponent->SetupAttachment(ThirdPersonGunStaticMesh);
		ThirdPersonFireSceneComponent->SetIsReplicated(true);
		ThirdPersonBulletCasingSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Third Person Bullet Casing Scene Component"));
		ThirdPersonBulletCasingSceneComponent->SetupAttachment(ThirdPersonGunStaticMesh);
		ThirdPersonBulletCasingSceneComponent->SetIsReplicated(true);
		GunStaticMesh->SetIsReplicated(true);
		GunStaticMesh->SetOnlyOwnerSee(true);
		GunStaticMesh->SetCollisionProfileName("NoCollision");
		GunStaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		ThirdPersonGunStaticMesh->SetOwnerNoSee(true);
		ThirdPersonGunStaticMesh->SetCollisionProfileName("NoCollision");
	}

	FireSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fire Scene Component"));
	FireSceneComponent->SetupAttachment(RootComponent, NAME_None);
	FireSceneComponent->SetIsReplicated(true);
	BulletCasingSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Casing Scene Component"));
	BulletCasingSceneComponent->SetupAttachment(RootComponent, NAME_None);
	BulletCasingSceneComponent->SetIsReplicated(true);
	PickupBoxCollision->SetupAttachment(RootComponent, NAME_None);
	FireBoxScene = CreateDefaultSubobject<USceneComponent>(TEXT("Fire Box Scene"));
	FireBoxScene->SetupAttachment(RootComponent, NAME_None);
	FireBoxScene->SetIsReplicated(true);
	DamageBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Damage Box Collision"));
	DamageBoxCollision->SetupAttachment(FireBoxScene, NAME_None);
	DamageBoxCollision->SetIsReplicated(true);

	CanShoot = true;
	FireMode = 0;
	UseBoxCollisionForDamage = false;
	IsShotgun = false;
	ShotgunAmountOfPellets = 8;
	ShotgunAmountOfPelletsShot = 0;
	FireFromBarrel = 0;
	BulletSpawnLocationOffset = FVector(0.0f, 0.0f, 0.0f);
	UseAimingSpreadMultiplier = true;
	SpreadAimingMultiplier = 0.25f;
	MinHipFireVerticalSpread = 0.0f;
	MaxHipFireVerticalSpread = 0.0f;
	MinHipFireHorizontalSpread = 0.0f;
	MaxHipFireHorizontalSpread = 0.0f;
	MinAimingFireVerticalSpread = 0.0f;
	MaxAimingFireVerticalSpread = 0.0f;
	MinAimingFireHorizontalSpread = 0.0f;
	MaxAimingFireHorizontalSpread = 0.0f;
	AutomaticallyAddPreDeterminedSpread = true;
	PreDeterminedSpreadIndex = 0;
	PreDeterminedAimingSpreadIndex = 0;
	PreDeterminedAimingSpreadWithMultiplierIndex = 0;
	AmmoInMagazine = 30;
	MaxAmmoInMagazine = 30;
	ReserveAmmo = 120;
	MaxReserveAmmo = 120;
	UseSharedCalibers = false;
	CaliberToUse = 0;
	InfiniteAmmo = 0;
	DoesOverheat = false;
	ProhibitFiringWhileCoolingDown = 0;
	ReachedMaxHeat = false;
	TimeToOverheat = 5.0f;
	TimeToCooldown = 5.0f;
	MaxHeat = 50.0f;
	CurrentHeat = 0.0f;
	IsOverheating = false;
	BulletCasingSpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	BulletCasingInheritsVelocity = true;
	MaxAmountOfBulletCasings = 30;
	ReloadSpeed = -1.0f;
	ReloadSpeed1 = -1.0f;
	WeaponSwitchTime = 0.0f;
	UseTwoWeaponSwitchAnimations = true;
	UseTwoReloadAnimations = true;
	UseTwoThirdPersonReloadAnimations = false;
	FireRate = 0.1f;
	ContinuousFireDamageRate = 0.005f;
	AmountOfShotsForBurst = 3;
	AmountOfBurstShotsFired = 0;
	FireRange = 100000000.0f;
	LaunchPhysicsObjects = true;
	LaunchObjectStrength = 7500.0f;
	LaunchObjectVelocityChange = false;
	IsExplosive = false;
	ExplosionIgnoreOwner = false;
	UseProjectile = false;
	ProjectileSpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ProjectileInheritsVelocity = false;
	TimeToDespawnProjectile = 10.0f;
	CollisionChannel = ECC_Visibility;
	DefaultDamage = 35.0f;
	ExplosiveDamageRadius = 500.0f;
	ExplosiveDoFullDamage = false;
	WasPickedup = false;
	AmountOfTimesPickedup = 0;
	WasPickedupBeginPlay = true;
	TimeToDespawnAfterDropped = 30.0f;
	SnapToSocket = 0;
	SpawnSmokeEffectWhenShooting = 1;
	AmountOfShotsToSpawnSmoke = 20;
	AmountOfTimeToCancelSmoke = 1.5f;
	DestroySmokeEffectWhenNotShooting = true;
	DestroySmokeSoundWhenNotShooting = true;
	TimeToDestroySmokeWhenNotShooting = 5.0f;
	TimeToDestroySmokeSoundWhenNotShooting = 5.0f;
	PercentageOfOverheatToSpawnSmoke = 0.75f;
	BulletsShotForSmokeEffect = 0;
	BulletHitMode = 0;
	InterpolateDamageBetweenRanges = true;
	PrintDistanceTraveled = false;
	BulletHitModeDelay = 0.0f;
	FireControllerVibrationTag = "Fire";
	BulletHitControllerVibrationTag = "Hit";
	SpawnFireSoundAttached = true;
	SpawnFireSound2DForOwner = true;
	SpawnFireSound2DForOwnerThirdPerson = true;
	HitEffectScale = FVector(1.0f, 1.0f, 1.0f);
	SpawnMuzzleFlashAttached = true;
	ReplicateMuzzleFlashLocation = false;
	UseFirstPersonRotationForThirdPersonMuzzleFlash = false;
	BulletWhizzingSoundVolumeBasedOnSpeed = true;
	UseActorClassesForHitMarkers = 2;
	UseFireArmsAnimation = 0;
	UseADS = 0;
	DivideAimingFOV = false;
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
	GunRelativeLocation = FVector(0.256082f, 0.156921, -1.547717);
	GunRelativeRotation = FRotator(4.474539, -10.125100, -2.840817);
	ThirdPersonGunRelativeLocation = FVector(0.256082f, 0.156921, -1.547717);
	ThirdPersonGunRelativeRotation = FRotator(4.474539, -10.125100, -2.840817);
	PlayerArmsRelativeLocation = FVector(-6.0f, -1.0f, -154.0f);
	PlayerArmsRelativeRotation = FRotator(0.0f, -5.0f, 0.0f);
	ADSArmsLocation = FVector(-13.523917, -15.856214, -150.601379);
	ADSArmsRotation = FRotator(2.190476, -7.458890, 2.467514);
	ZoomArmsLocation = FVector(-10.412415, -1.490295, -154.332474);
	ZoomArmsRotation = FRotator(1.579085, -6.870842, 0.000002);
	SocketName = "GripPoint";
	ThirdPersonSocketName = "GripPoint";
	CanCrumbleDestructibleMeshes = false;
	CrumbleDestructibleMeshesWithEveryShotgunPellet = false;
	DestructionSphereSize = FVector(0.25f, 0.25f, 0.25f);
	SwitchedFireToServer = false;
}

UPrimitiveComponent* AMultiplayerGun::GetGunMesh()
{
	if (UseSkeletalMesh == true)
	{
		return GunSkeletalMesh;
	}
	else
	{
		return GunStaticMesh;
	}
}

UPrimitiveComponent* AMultiplayerGun::GetThirdPersonGunMesh()
{
	if (UseSkeletalMesh == true)
	{
		return ThirdPersonGunSkeletalMesh;
	}
	else
	{
		return ThirdPersonGunStaticMesh;
	}
}

void AMultiplayerGun::CheckForOwner()
{
	if (WasPickedup == true)
	{
		bool GetDestroyed;

		if (OwningPlayer)
		{
			if (GetOwningPlayerCast())
			{
				if (GetOwningPlayerCast()->GetHealthComponent()->GetHealth() >= 1)
				{
					GetDestroyed = false;
				}
				else
				{
					GetDestroyed = true;
				}
			}
			else
			{
				GetDestroyed = true;
			}
		}
		else
		{
			GetDestroyed = true;
		}

		if (GetDestroyed == true)
		{
			GetWorldTimerManager().ClearTimer(CheckForOwnerTimerHandle);

			DestroySelf();
		}
	}
}

AMultiplayerCharacter* AMultiplayerGun::GetOwningPlayerCast()
{
	if (WasPickedup == false)
	{
		SetOwner(nullptr);
		OwningPlayer = nullptr;
		OwningPlayerCast = nullptr;
		return nullptr;
	}
	
	if (!OwningPlayerCast && GetOwningPlayer())
	{
		OwningPlayerCast = Cast<AMultiplayerCharacter>(GetOwningPlayer());
	}

	return OwningPlayerCast;
}

void AMultiplayerGun::DestroySelf()
{
	Destroy();
}

USceneComponent* AMultiplayerGun::GetFireSceneToUse()
{
	USceneComponent* FireSceneToUse;

	if (GetUsingThirdPerson() == true)
	{
		FireSceneToUse = ThirdPersonFireSceneComponent;
	}
	else
	{
		FireSceneToUse = FireSceneComponent;
	}

	if (GetOwningPlayer())
	{
		if (GetOwningPlayer()->IsLocallyControlled() || ReplicateMuzzleFlashLocation == true)
		{
			return FireSceneToUse;
		}
		else
		{
			return ThirdPersonFireSceneComponent;
		}
	}
	else
	{
		return ThirdPersonFireSceneComponent;
	}
}

USceneComponent* AMultiplayerGun::GetBulletCasingSceneToUse()
{
	USceneComponent* BulletCasingSceneToUse;

	if (GetUsingThirdPerson() == true)
	{
		BulletCasingSceneToUse = ThirdPersonBulletCasingSceneComponent;
	}
	else
	{
		BulletCasingSceneToUse = BulletCasingSceneComponent;
	}

	if (GetOwningPlayer())
	{
		if (GetOwningPlayer()->IsLocallyControlled())
		{
			return BulletCasingSceneToUse;
		}
		else
		{
			return ThirdPersonBulletCasingSceneComponent;
		}
	}
	else
	{
		return ThirdPersonBulletCasingSceneComponent;
	}
}

void AMultiplayerGun::SetUsingThirdPerson(bool NewUsingThirdPerson)
{
	if (HasAuthority())
	{
		MulticastSetUsingThirdPerson(NewUsingThirdPerson);
	}
	else
	{
		ServerSetUsingThirdPerson(NewUsingThirdPerson);
	}
}

void AMultiplayerGun::ServerSetUsingThirdPerson_Implementation(bool NewUsingThirdPerson)
{
	MulticastSetUsingThirdPerson(NewUsingThirdPerson);
}

void AMultiplayerGun::MulticastSetUsingThirdPerson_Implementation(bool NewUsingThirdPerson)
{
	UsingThirdPerson = NewUsingThirdPerson;
}

bool AMultiplayerGun::GetUsingThirdPerson()
{
	return UsingThirdPerson;
}

void AMultiplayerGun::SetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo)
{
	SetWasPickedup_BP(Pickedup, ComponentToAttachTo);

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
	UPrimitiveComponent* GunMeshComponent = GetGunMesh();
	UPrimitiveComponent* ThirdPersonGunMeshComponent = GetThirdPersonGunMesh();

	if (Pickedup == true)
	{
		if (GetOwningPlayer())
		{
			GetWorldTimerManager().ClearTimer(DespawnTimerHandle);

			if (GetOwningPlayerCast())
			{
				ApplyPerspective(GetOwningPlayerCast()->GetUsingThirdPerson());
			}
			else
			{
				if (HasAuthority())
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Server GetOwningPlayerCast Failed MultiplayerGun.cpp:MulticastSetWasPickedup");
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Client GetOwningPlayerCast Failed MultiplayerGun.cpp:MulticastSetWasPickedup");
				}
			}

			PickupBoxCollision->SetGenerateOverlapEvents(false);

			AmountOfTimesPickedup++;

			if (ComponentToAttachTo)
			{
				if (GunMeshComponent)
				{
					GunMeshComponent->SetSimulatePhysics(false);
					GunMeshComponent->SetCollisionProfileName("NoCollision");
					GunMeshComponent->SetOnlyOwnerSee(true);
				}

				if (ThirdPersonGunMeshComponent)
				{
					ThirdPersonGunMeshComponent->SetHiddenInGame(false);
					ThirdPersonGunMeshComponent->SetVisibility(true);
				}

				if (SnapToSocket == 0)
				{
					AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::KeepWorldTransform, SocketName);

					SetActorRelativeLocation(GunRelativeLocation);
					SetActorRelativeRotation(GunRelativeRotation);

					if (GetOwningPlayerCast())
					{
						if (GetOwningPlayerCast()->GetPlayerModelMesh())
						{
							ThirdPersonGunMeshComponent->AttachToComponent(GetOwningPlayerCast()->GetPlayerModelMesh(), FAttachmentTransformRules::KeepWorldTransform, ThirdPersonSocketName);

							ThirdPersonGunMeshComponent->SetRelativeLocation(ThirdPersonGunRelativeLocation);
							ThirdPersonGunMeshComponent->SetRelativeRotation(ThirdPersonGunRelativeRotation);
						}
					}
				}
				else if (SnapToSocket == 1)
				{
					AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

					if (GetOwningPlayerCast())
					{
						if (GetOwningPlayerCast()->GetPlayerModelMesh())
						{
							ThirdPersonGunMeshComponent->AttachToComponent(GetOwningPlayerCast()->GetPlayerModelMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ThirdPersonSocketName);
						}
					}
				}
				else
				{
					AttachToComponent(ComponentToAttachTo, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);

					if (GetOwningPlayerCast())
					{
						if (GetOwningPlayerCast()->GetPlayerModelMesh())
						{
							ThirdPersonGunMeshComponent->AttachToComponent(GetOwningPlayerCast()->GetPlayerModelMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, ThirdPersonSocketName);
						}
					}
				}

				if (GetOwningPlayerCast())
				{
					GetOwningPlayerCast()->SetOverlappingInteractable(false);
					GetOwningPlayerCast()->SetInteractableBeingOverlapped(nullptr);
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "OwningPlayer Invalid MultiplayerGun.cpp:SetWasPickedup");
		}
	}
	else
	{
		StopFiring();
		SetOwningPlayer(nullptr, 4);

		if (ThirdPersonGunMeshComponent)
		{
			ThirdPersonGunMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform, NAME_None);
			ThirdPersonGunMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			ThirdPersonGunMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

			ThirdPersonGunMeshComponent->SetHiddenInGame(true);
			ThirdPersonGunMeshComponent->SetVisibility(false);
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (GunMeshComponent)
		{
			GunMeshComponent->SetCollisionProfileName("BlockAllDynamic");
			GunMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			GunMeshComponent->SetSimulatePhysics(true);
			GunMeshComponent->SetOnlyOwnerSee(false);
		}

		SetActorHiddenInGame(false);
		SetOwner(nullptr);
		SetOwningPlayer(nullptr);

		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerGun::SetPickupCollisionEnabled);

		if (TimeToDespawnAfterDropped > 0 && AmountOfTimesPickedup > 0)
		{
			GetWorldTimerManager().SetTimer(DespawnTimerHandle, this, &AMultiplayerGun::DestroySelf, TimeToDespawnAfterDropped, false, TimeToDespawnAfterDropped);
		}
	}
}

bool AMultiplayerGun::GetWasPickedup()
{
	return WasPickedup;
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

void AMultiplayerGun::SetOwningPlayer(APawn* NewOwningPlayer, int ReplicationMethod)
{
	if (ReplicationMethod == 1 || ReplicationMethod == 3)
	{
		ServerSetOwningPlayer(NewOwningPlayer, ReplicationMethod);
	}
	else if (ReplicationMethod == 2)
	{
		MulticastSetOwningPlayer(NewOwningPlayer);
	}
	else if (ReplicationMethod == 4)
	{
		if (HasAuthority())
		{
			ServerSetOwningPlayer(NewOwningPlayer, ReplicationMethod);
		}
		else
		{
			MulticastSetOwningPlayer(NewOwningPlayer);
		}
	}
	else
	{
		SetOwner(NewOwningPlayer);
		OwningPlayer = NewOwningPlayer;
		OwningPlayerCast = nullptr;
	}
}

void AMultiplayerGun::ServerSetOwningPlayer_Implementation(APawn* NewOwningPlayer, int ReplicationMethod)
{
	if (ReplicationMethod >= 3)
	{
		MulticastSetOwningPlayer(NewOwningPlayer);
	}
	else
	{
		SetOwner(NewOwningPlayer);
		OwningPlayer = NewOwningPlayer;
		OwningPlayerCast = nullptr;
	}
}

void AMultiplayerGun::MulticastSetOwningPlayer_Implementation(APawn* NewOwningPlayer)
{
	SetOwner(NewOwningPlayer);
	OwningPlayer = NewOwningPlayer;
	OwningPlayerCast = nullptr;
}

APawn* AMultiplayerGun::GetOwningPlayer()
{
	if (!OwningPlayer && GetOwner())
	{
		OwningPlayer = Cast<APawn>(GetOwner());
	}

	return OwningPlayer;
}

void AMultiplayerGun::FireInput()
{
	if (AmmoInMagazine > 0 || InfiniteAmmo == 2)
	{
		if (CanShoot == true && GetOwningPlayer() && WasPickedup == true && CurrentHeat < MaxHeat)
		{
			if ((ProhibitFiringWhileCoolingDown == 1 && CurrentHeat > 0 && ReachedMaxHeat == true) || (ProhibitFiringWhileCoolingDown == 2 && CurrentHeat > 0))
			{
				return;
			}

			if (IsShotgun == false || (IsShotgun == true && ShotgunAmountOfPelletsShot == 0))
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
				}
			}
			else if (FireMode == 1 && CurrentHeat < MaxHeat - 1)
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
	}
}

void AMultiplayerGun::Fire()
{
	if (WasPickedup == true && GetOwningPlayer())
	{
		if (!HasAuthority())
		{
			ServerFire();
		}

		if (IsShotgun == false && SwitchedFireToServer == false)
		{
			int TempBurstShots = AmountOfBurstShotsFired;

			if (!HasAuthority())
			{
				TempBurstShots++;
			
				ClientFire();

				if (IsShotgun == false && TempBurstShots >= AmountOfShotsForBurst)
				{
					if (FireMode != 3)
					{
						if (GetFireSceneToUse() && GetOwningPlayerCast())
						{
							if (FireSound)
							{
								if (SpawnFireSoundAttached == true)
								{
									if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
									{
										UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
									}
									else
									{
										if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
										{
											UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
										}
										else
										{
											UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, FireSoundAttenuationOverride);
										}
									}
								}
								else
								{
									if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
									{
										UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
									}
									else
									{
										if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
										{
											UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
										}
										else
										{
											UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, FireSoundAttenuationOverride);
										}
									}
								}
							}

							if (MuzzleFlash)
							{
								if (SpawnMuzzleFlashAttached == true)
								{
									UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, GetFireSceneToUse());
								}
								else
								{
									UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, GetFireSceneToUse()->GetComponentLocation());
								}
							}
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FireSceneToUse Invalid MultiplayerGun.cpp:Fire");
						}
					}
				}
			}
		
			if ((FireMode == 2 && TempBurstShots >= AmountOfShotsForBurst) || (FireMode == 2 && AmmoInMagazine <= 0 && InfiniteAmmo != 2))
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

		if ((AmmoInMagazine > 0 && CurrentHeat < MaxHeat) || (IsShotgun == true && ShotgunAmountOfPelletsShot > 0) || InfiniteAmmo == 2)
		{
			if (CanShoot == true || FireMode == 1 || FireMode == 2 || IsShotgun == true)
			{
				GetWorldTimerManager().ClearTimer(DestroySmokeEffectTimerHandle);
				GetWorldTimerManager().ClearTimer(CancelSmokeEffectTimerHandle);

				Fire_BP();

				if (HasAuthority())
				{
					ServerFire_BP();
				}

				FVector FireLocation;
				FRotator FireRotation;

				FVector TempVector = FVector::ZeroVector;
				FRotator TempRotator = FRotator::ZeroRotator;

				switch (FireFromBarrel)
				{
				case 1:
					FireLocation = FireSceneComponent->GetComponentLocation();
                    						
					if (GetOwningPlayerCast())
					{
						FireRotation = GetOwningPlayerCast()->ReplicatedCameraRotation;
					}
					else
					{
						GetOwningPlayer()->GetActorEyesViewPoint(TempVector, FireRotation);

						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid MultiplayerGun.cpp:Fire 1");
					}
					
					break;
				case 2:
					FireRotation = FireSceneComponent->GetComponentRotation();
					
					if (GetOwningPlayerCast())
					{
						FireLocation = GetOwningPlayerCast()->ReplicatedCameraLocation;
					}
					else
					{
						GetOwningPlayer()->GetActorEyesViewPoint(FireLocation, TempRotator);

						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid MultiplayerGun.cpp:Fire 2");
					}
					
					break;
				case 3:
					FireLocation = FireSceneComponent->GetComponentLocation();
					FireRotation = FireSceneComponent->GetComponentRotation();
					break;
				default:
					if (GetOwningPlayerCast())
					{
						FireLocation = GetOwningPlayerCast()->ReplicatedCameraLocation;
						FireRotation = GetOwningPlayerCast()->ReplicatedCameraRotation;
					}
					else
					{
						GetOwningPlayer()->GetActorEyesViewPoint(FireLocation, FireRotation);

						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid MultiplayerGun.cpp:Fire 3");
					}
					
					break;
				}

				FireLocation += GetOwningPlayer()->GetActorRightVector() * BulletSpawnLocationOffset.X;
				FireLocation += GetOwningPlayer()->GetActorForwardVector() * BulletSpawnLocationOffset.Y;
				FireLocation += GetOwningPlayer()->GetActorUpVector() * BulletSpawnLocationOffset.Z;

				FVector TraceDirection;

				if (GetOwningPlayerCast())
				{
					if (GetOwningPlayerCast()->GetIsAiming() == true)
					{
						if (UseAimingSpreadMultiplier == true)
						{
							if (PreDeterminedAimingSpreadWithMultiplier.IsValidIndex(PreDeterminedAimingSpreadWithMultiplierIndex))
							{
								FireRotation += PreDeterminedAimingSpreadWithMultiplier[PreDeterminedAimingSpreadWithMultiplierIndex];
							}

							PreDeterminedAimingSpreadWithMultiplierIndex++;

							if (PreDeterminedAimingSpreadWithMultiplierIndex >= PreDeterminedAimingSpreadWithMultiplier.Num() - 1)
							{
								PreDeterminedAimingSpreadWithMultiplierIndex = 0;
							}
						}
						else
						{
							if (PreDeterminedAimingSpread.IsValidIndex(PreDeterminedAimingSpreadIndex))
							{
								FireRotation += PreDeterminedAimingSpread[PreDeterminedAimingSpreadIndex];
							}

							PreDeterminedAimingSpreadIndex++;

							if (PreDeterminedAimingSpreadIndex >= PreDeterminedAimingSpread.Num() - 1)
							{
								PreDeterminedAimingSpreadIndex = 0;
							}
						}
					}
					else
					{
						if (PreDeterminedSpread.IsValidIndex(PreDeterminedSpreadIndex))
						{
							FireRotation += PreDeterminedSpread[PreDeterminedSpreadIndex];
						}

						PreDeterminedSpreadIndex++;

						if (PreDeterminedSpreadIndex >= PreDeterminedSpread.Num() - 1)
						{
							PreDeterminedSpreadIndex = 0;
						}
					}
				}

				TraceDirection = FireRotation.Vector();

				// The reason to add this offset is because the GunHitEffectsReplication function won't execute when the gun is a projectile and the fire location is same as last time the gun was fired
				GunHitEffectsReplication.FireLocation = GetFireSceneToUse()->GetComponentLocation() + FVector(FMath::RandRange(-0.25f, 0.25f), FMath::RandRange(-0.25f, 0.25f), FMath::RandRange(-0.25f, 0.25f));

				if (UseProjectile == false)
				{
					FVector TraceEnd = FireLocation + (TraceDirection * FireRange);

					FCollisionQueryParams QueryParams;
					QueryParams.AddIgnoredActor(this);
					QueryParams.AddIgnoredActor(GetOwningPlayer());
					QueryParams.bTraceComplex = true;
					QueryParams.bReturnPhysicalMaterial = true;

					if (AdditionalActorsToIgnore.Num() >= 1)
					{
						for (auto& ActorToIgnore : AdditionalActorsToIgnore)
						{
							QueryParams.AddIgnoredActor(ActorToIgnore);
						}
					}

					FHitResult Hit;
					if (GetWorld()->LineTraceSingleByChannel(Hit, FireLocation, TraceEnd, CollisionChannel, QueryParams) && UseBoxCollisionForDamage == false)
					{
						AActor* HitActor = Hit.GetActor();
						EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
						UParticleSystem* HitEffect;
						UPhysicalMaterial* HitSurface = Hit.PhysMaterial.Get();

						if (HasAuthority())
						{
							CallBulletHitDelegate(HitActor, HitSurface);
						}

						if (IsExplosive == true)
						{
							HitEffect = DefaultHitEffect;
						}
						else
						{
							if (HasAuthority())
							{
								if (GetOwningPlayerCast())
								{
									if (AMultiplayerCharacter* EnemyCast = Cast<AMultiplayerCharacter>(HitActor))
									{
										if (EnemyCast->GetHealthComponent())
										{
											if (EnemyCast->GetHealthComponent()->GetHealth() > 0)
											{
												if (UseActorClassesForHitMarkers == 0 && HitActor)
												{
													GetOwningPlayerCast()->ShowHitMarker(HitActor, nullptr);
												}
												else if (UseActorClassesForHitMarkers == 1 && HitSurface)
												{
													GetOwningPlayerCast()->ShowHitMarker(nullptr, HitSurface);
												}
												else if (UseActorClassesForHitMarkers == 2 && HitActor && HitSurface)
												{
													GetOwningPlayerCast()->ShowHitMarker(HitActor, HitSurface);
												}
											}
										}
									}
								}
							}
							
							int HitEffectIndex;
							TArray<UPhysicalMaterial*> HitEffectPhysicalMaterials;
							TArray<UParticleSystem*> HitParticleSystems;

							HitEffects.GenerateKeyArray(HitEffectPhysicalMaterials);
							HitEffects.GenerateValueArray(HitParticleSystems);

							if (HitEffectPhysicalMaterials.Contains(HitSurface))
							{
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
						}

						GunHitEffectsReplication.HitLocation = Hit.ImpactPoint;
						GunHitEffectsReplication.HitRotation = Hit.ImpactNormal.Rotation();
						GunHitEffectsReplication.HitEffect = HitEffect;
						GunHitEffectsReplication.HitResult = Hit;

						if (HitEffect)
						{
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), HitEffectScale);
						}

						USoundBase* ChosenHitSound;

						if (IsExplosive == false)
						{
							int HitSoundIndex;
							TArray<USoundBase*> HitSounds;
							TArray<UPhysicalMaterial*> HitPhysicalMaterials;

							BulletHitSounds.GenerateKeyArray(HitSounds);
							BulletHitSounds.GenerateValueArray(HitPhysicalMaterials);

							UPhysicalMaterial* HitPhysicalMaterial = HitSurface;

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
								IgnoredActors.Add(GetOwningPlayer());
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

							if (BulletHitMode != 2)
							{
								if (UGameplayStatics::ApplyRadialDamage(GetWorld(), DefaultDamage, HitLocation, ExplosiveDamageRadius, DamageType, IgnoredActors, this, GetInstigatorController(), ExplosiveDoFullDamage, CollisionChannel))
								{
									if (HasAuthority())
									{
										FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(ApplyRadialDamage),  false, nullptr);

										SphereParams.AddIgnoredActors(IgnoredActors);

										TArray<FOverlapResult> Overlaps;
										if (UWorld* World = GEngine->GetWorldFromContextObject(GetWorld(), EGetWorldErrorMode::LogAndReturnNull))
										{
											World->OverlapMultiByObjectType(Overlaps, Hit.ImpactPoint, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(ExplosiveDamageRadius), SphereParams);
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

												FVector const LineTraceEnd = VictimComp->Bounds.Origin;
												FVector TraceStart = Hit.ImpactPoint;
												if (Hit.ImpactPoint == LineTraceEnd)
												{
													TraceStart.Z += 0.01f;
												}

												FHitResult OutHitResult;
												
												FVector const FakeHitLoc = VictimComp->GetComponentLocation();
												FVector const FakeHitNorm = (Hit.ImpactPoint - FakeHitLoc).GetSafeNormal();
												OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp.Get(), FakeHitLoc, FakeHitNorm);

												AActor* ExplosiveHitActor = VictimComp.Get()->GetOwner();

												if (GetOwningPlayerCast())
												{
													if (AMultiplayerCharacter* EnemyCast = Cast<AMultiplayerCharacter>(ExplosiveHitActor))
													{
														if (EnemyCast->GetHealthComponent())
														{
															if (EnemyCast->GetHealthComponent()->GetHealth() > 0)
															{
																if (ExplosiveHitActor)
																{
																	GetOwningPlayerCast()->ShowHitMarker(ExplosiveHitActor, nullptr);
																	break;
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
						else
						{
							UMaterialInterface* ChosenDecal;

							int DecalIndex;
							TArray<UMaterialInterface*> DecalMaterials;
							TArray<UPhysicalMaterial*> DecalPhysicalMaterials;

							BulletHitDecals.GenerateKeyArray(DecalMaterials);
							BulletHitDecals.GenerateValueArray(DecalPhysicalMaterials);

							UPhysicalMaterial* HitPhysicalMaterial = HitSurface;

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

							float DamageToApply;

							int DamageIndex;
							TArray<UPhysicalMaterial*> DamagePhysicalMaterials;
							TArray<float> DamageAmounts;

							Damage.GenerateKeyArray(DamagePhysicalMaterials);
							Damage.GenerateValueArray(DamageAmounts);

							if (DamagePhysicalMaterials.Contains(HitSurface))
							{
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

							float DistanceTraveled = (FireLocation - Hit.ImpactPoint).Size();

							if (PrintDistanceTraveled == true)
							{
								PrintDistanceTraved_BP(DistanceTraveled);
							}

							if (DamageFalloffMultiplierAtRange.Num() > 0)
							{
								TArray<float> Distances;
								TArray<float> Damages;
								
								DamageFalloffMultiplierAtRange.GenerateKeyArray(Distances);
								DamageFalloffMultiplierAtRange.GenerateValueArray(Damages);

								Algo::Reverse(Distances);
								Algo::Reverse(Damages);

								bool AppliedDamageFalloff = false;

								for (int32 Index = 0; Index != Distances.Num(); ++Index)
								{
									if (Distances.IsValidIndex(Index) && AppliedDamageFalloff == false)
									{
										if (DistanceTraveled >= Distances[Index])
										{
											if (Index != 0 && InterpolateDamageBetweenRanges == true)
											{
												DamageToApply *= UKismetMathLibrary::Ease(Damages[Index], Damages[Index - 1], UKismetMathLibrary::MapRangeClamped(DistanceTraveled, Distances[Index], Distances[Index -1], 0.0, 1.0), EEasingFunc::Linear);
											}
											else
											{
												DamageToApply *= Damages[Index];
											}
											
											AppliedDamageFalloff = true;
										}
									}
								}
							}

							if (LaunchPhysicsObjects == true && LaunchObjectStrength > 0)
							{
								if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
								{
									if (HitComponent->IsSimulatingPhysics() == true)
									{
										HitComponent->AddImpulse(TraceDirection * LaunchObjectStrength, NAME_None, LaunchObjectVelocityChange);
									}
								}
							}

							if (BulletHitMode != 2)
							{
								UGameplayStatics::ApplyPointDamage(HitActor, DamageToApply, TraceDirection, Hit, GetInstigatorController(), this, DamageType);
							}
						}

						if (DestructionSphereToSpawn && CanCrumbleDestructibleMeshes == true)
						{
							if (IsShotgun == false || CrumbleDestructibleMeshesWithEveryShotgunPellet == true)
							{
								FActorSpawnParameters SpawnParams;
								SpawnParams.bNoFail = true;
								SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

								GetWorld()->SpawnActor<AActor>(DestructionSphereToSpawn, FTransform(Hit.ImpactPoint.Rotation(), Hit.ImpactPoint + (Hit.ImpactNormal * -3.0f), DestructionSphereSize), SpawnParams);
							}
						}

						if (IsShotgun == true)
						{
							ShotgunPelletHitLocations.Add(Hit.ImpactPoint + (Hit.ImpactNormal * -3.0f));
						}

						if (BulletHitMode != 0 && HasAuthority())
						{
							if (BulletHitModeDelay <= 0.0f)
							{
								ExecuteHitFunction(GetOwningPlayer(), HitActor);
							}
							else
							{
								BulletHitModeTimerDelegate.BindUFunction(this, "ExecuteHitFunction", GetOwningPlayer(), HitActor);
								GetWorldTimerManager().SetTimer(BulletHitModeTimerHandle, BulletHitModeTimerDelegate, BulletHitModeDelay, false, BulletHitModeDelay);
							}
						}
					}
					else if (UseBoxCollisionForDamage == true)
					{
						TArray<AActor*> OverlappingActors;
						DamageBoxCollision->GetOverlappingActors(OverlappingActors);

						for (auto& Actor : OverlappingActors)
						{
							if (Actor)
							{
								FCollisionQueryParams QueryParams1;
								QueryParams1.AddIgnoredActor(this);
								QueryParams1.AddIgnoredActor(GetOwningPlayer());
								QueryParams1.AddIgnoredActor(Actor);

								if (GetOwningPlayerCast())
								{
									for (auto& Weapon : GetOwningPlayerCast()->GetAllWeapons())
									{
										QueryParams1.AddIgnoredActor(Weapon);
									}
								}

								FHitResult HitResult;
								if (Actor != GetOwningPlayer() && BulletHitMode != 2)
								{
									GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), Actor->GetActorLocation(), ECC_Visibility, QueryParams1);

									if (HitResult.bBlockingHit == false || OverlappingActors.Contains(HitResult.GetActor()))
									{
										UGameplayStatics::ApplyDamage(Actor, DefaultDamage, GetOwningPlayer()->GetInstigatorController(), GetOwningPlayer(), DamageType);

										if (HasAuthority())
										{
											if (GetOwningPlayerCast())
											{
												if (AMultiplayerCharacter* EnemyCast = Cast<AMultiplayerCharacter>(Actor))
												{
													if (EnemyCast->GetHealthComponent())
													{
														if (EnemyCast->GetHealthComponent()->GetHealth() > 0)
														{
															if (UseActorClassesForHitMarkers == 0 && Actor)
															{
																GetOwningPlayerCast()->ShowHitMarker(Actor, nullptr);
															}
														}
													}
												}
											}
										}
									}
								}

								if (BulletHitMode != 0 && HasAuthority())
								{
									if (BulletHitModeDelay <= 0.0f)
									{
										ExecuteHitFunction(GetOwningPlayer(), Actor);
									}
									else
									{
										BulletHitModeTimerDelegate.BindUFunction(this, "ExecuteHitFunction", GetOwningPlayer(), Actor);
										GetWorldTimerManager().SetTimer(BulletHitModeTimerHandle, BulletHitModeTimerDelegate, BulletHitModeDelay, false, BulletHitModeDelay);
									}
								}
							}
						}
					}

					// ************* Debug *************

					if (ShowBulletPath == true)
					{
						DrawDebugLine(GetWorld(), FireLocation, TraceEnd, BulletPathColor, BulletPathPersists, BulletPathDuration);
					}

					// ************* Debug *************
				}
				else if (SwitchedFireToServer == false)
				{
					SpawnProjectile(FireLocation, FireRotation, TraceDirection);
				}

				if (IsShotgun == false)
				{
					if (FireMode != 3)
					{
						if (GetFireSceneToUse() && GetOwningPlayerCast())
						{
							if (FireSound)
							{
								if (SpawnFireSoundAttached == true)
								{
									if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
									{
										UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
									}
									else
									{
										if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
										{
											UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
										}
										else
										{
											UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, FireSoundAttenuationOverride);
										}
									}
								}
								else
								{
									if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
									{
										UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
									}
									else
									{
										if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
										{
											UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
										}
										else
										{
											UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, FireSoundAttenuationOverride);
										}
									}
								}
							}

							if (MuzzleFlash)
							{
								if (SpawnMuzzleFlashAttached == true)
								{
									UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, GetFireSceneToUse());
								}
								else
								{
									UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, GetFireSceneToUse()->GetComponentLocation());
								}
							}
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FireSceneToUse Invalid MultiplayerGun.cpp:Fire");
						}
					}

					if (SwitchedFireToServer == false)
					{
						ClientFire();
					}

					if (FireMode != 3)
					{
						CanShoot = false;
					}

					if (HasAuthority() || UseProjectile == true)
					{
						SpawnBulletCasing();
					}

					if (DoesOverheat == true && InfiniteAmmo == 0)
					{
						if (IsOverheating == false)
						{
							IsOverheating = true;

							Overheat_BP();
						}
					}
					else if (InfiniteAmmo != 2)
					{
						AmmoInMagazine--;
					}
				}

				if (DoesOverheat == false && InfiniteAmmo != 2)
				{
					if (GetOwningPlayerCast())
					{
						if (AmmoInMagazine <= 0)
						{
							GetOwningPlayerCast()->Reload();
						}
					}
				}

				FireTimerDelegate.BindUFunction(this, FName("SetCanShoot"), true);
				GetWorldTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, FireRate, false, FireRate);
			}
		}
		else if (CurrentHeat >= MaxHeat)
		{
			StopFiring();
		}
	}
}

void AMultiplayerGun::ServerFire_Implementation()
{
	SwitchedFireToServer = true;

	Fire();
}

void AMultiplayerGun::ClientFire_Implementation()
{
	ClientFire_BP();

	if (UseSkeletalMesh == true && GunSkeletalMesh && FireGunAnimation)
	{
		GunSkeletalMesh->PlayAnimation(FireGunAnimation, false);
	}

	if (GetOwningPlayerCast())
	{
		if (GetOwningPlayerCast()->ArmsMesh && GetOwningPlayerCast()->GetIsADSing() == false)
		{
			if (UseFireArmsAnimation != 1 && FireArmsAnimationMontage)
			{
				if (UAnimInstance* ArmsAnimationInstance = GetOwningPlayerCast()->ArmsMesh->GetAnimInstance())
				{
					ArmsAnimationInstance->Montage_Play(FireArmsAnimationMontage);
				}
			}

			if (UseFireArmsAnimation != 0 && FireArmsAnimation)
			{
				GetOwningPlayerCast()->ArmsMesh->PlayAnimation(FireArmsAnimation, false);
				GetOwningPlayerCast()->SetArmsAnimationMode(FireArmsAnimation->GetPlayLength());
			}
		}
	}

	if (FireCameraShake && GetOwningPlayer())
	{
		if (APlayerController* OwningController = Cast<APlayerController>(GetOwningPlayer()->GetController()))
		{
			OwningController->ClientStartCameraShake(FireCameraShake);

			if (FireMode != 3)
			{
				if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(OwningController))
				{
					if (FireControllerVibration)
					{
						ControllerCast->VibrateController(FireControllerVibration, FireControllerVibrationTag);
					}
				}
			}
		}
	}
	
	AddRecoil_BP();

	if (SpawnSmokeEffectWhenShooting == 1)
	{
		BulletsShotForSmokeEffect++;

		if (BulletsShotForSmokeEffect >= AmountOfShotsToSpawnSmoke)
		{
			SpawnSmokeEffect_BP();
		}
	}
}

void AMultiplayerGun::AddPredeterminedSpread()
{
	if (AutomaticallyAddPreDeterminedSpread == true)
	{
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.46, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.63, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.1, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.36, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.9, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.87, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.3, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.84, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.2, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.27, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.78, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.95, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.37, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.4, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.7, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));
		PreDeterminedSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinHipFireVerticalSpread, MaxHipFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.85, 0.0, 1.0, MinHipFireHorizontalSpread, MaxHipFireHorizontalSpread), 0.0));

		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.46, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.63, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.1, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.36, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.9, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.87, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.3, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.84, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.2, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.27, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.78, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.95, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.37, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.4, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.7, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));
		PreDeterminedAimingSpread.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinAimingFireVerticalSpread, MaxAimingFireVerticalSpread), UKismetMathLibrary::MapRangeClamped(0.85, 0.0, 1.0, MinAimingFireHorizontalSpread, MaxAimingFireHorizontalSpread), 0.0));

		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.5, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.46, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.63, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.1, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.36, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.9, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.87, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.3, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.32, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.84, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.2, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.27, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.78, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.95, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.37, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.4, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(1.0, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.7, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.67, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.0, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.15, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
		PreDeterminedAimingSpreadWithMultiplier.Add(FRotator(UKismetMathLibrary::MapRangeClamped(0.45, 0.0, 1.0, MinHipFireVerticalSpread * SpreadAimingMultiplier, MaxHipFireVerticalSpread * SpreadAimingMultiplier), UKismetMathLibrary::MapRangeClamped(0.85, 0.0, 1.0, MinHipFireHorizontalSpread * SpreadAimingMultiplier, MaxHipFireHorizontalSpread * SpreadAimingMultiplier), 0.0));
	}
}

void AMultiplayerGun::ExecuteHitFunction_Implementation(AActor* ParentPlayer, AActor* HitActor)
{

}

void AMultiplayerGun::CallBulletHitDelegate(AActor* HitActor, UPhysicalMaterial* HitSurface)
{
	OnBulletHit.Broadcast(HitActor, HitSurface);
}

void AMultiplayerGun::ShotgunFire()
{
	if (CurrentHeat < MaxHeat || InfiniteAmmo == 2)
	{
		if (!HasAuthority())
		{
			ServerShotgunFire();
		}

		ClientFire();

		ShotgunPelletHitLocations.Empty();

		if ((FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsForBurst) || (FireMode == 2 && AmmoInMagazine <= 0 && InfiniteAmmo != 2))
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

		if (CrumbleDestructibleMeshesWithEveryShotgunPellet == false)
		{
			FVector AverageHitLocation(0, 0, 0);

			if (ShotgunPelletHitLocations.Num() > 0)
			{
				for (auto& Location : ShotgunPelletHitLocations)
				{
					AverageHitLocation += Location;
				}

				AverageHitLocation /= ShotgunPelletHitLocations.Num();

				FActorSpawnParameters SpawnParams;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				GetWorld()->SpawnActor<AActor>(DestructionSphereToSpawn, FTransform(AverageHitLocation.Rotation(), AverageHitLocation, DestructionSphereSize), SpawnParams);
			}
		}

		if (FireMode != 3)
		{
			if (GetFireSceneToUse() && GetOwningPlayerCast())
			{
				if (FireSound)
				{
					if (SpawnFireSoundAttached == true)
					{
						if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
						{
							UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
						}
						else
						{
							if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
							{
								UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
							}
							else
							{
								UGameplayStatics::SpawnSoundAttached(FireSound, GetFireSceneToUse(), NAME_None, FVector(ForceInit), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false, 1, 1, 0, FireSoundAttenuationOverride);
							}
						}
					}
					else
					{
						if (((SpawnFireSound2DForOwner == true && GetOwningPlayerCast()->GetUsingThirdPerson() == false) || (SpawnFireSound2DForOwnerThirdPerson == true && GetOwningPlayerCast()->GetUsingThirdPerson() == true)) && GetOwningPlayerCast()->IsLocallyControlled() == true)
						{
							UGameplayStatics::SpawnSound2D(GetWorld(), FireSound);
						}
						else
						{
							if (GetOwningPlayerCast()->GetUsingThirdPerson() == true)
							{
								UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, ThirdPersonFireSoundAttenuationOverride);
							}
							else
							{
								UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetFireSceneToUse()->GetComponentLocation(), FRotator::ZeroRotator, 1, 1, 0, FireSoundAttenuationOverride);
							}
						}
					}
				}

				if (MuzzleFlash)
				{
					if (SpawnMuzzleFlashAttached == true)
					{
						UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, GetFireSceneToUse());
					}
					else
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, GetFireSceneToUse()->GetComponentLocation());
					}
				}
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FireSceneToUse Invalid MultiplayerGun.cpp:Fire");
			}
		}

		if (HasAuthority())
		{
			SpawnBulletCasing();
		}

		ShotgunAmountOfPelletsShot = 0;

		CanShoot = false;

		if (DoesOverheat == true && InfiniteAmmo == 0)
		{
			if (IsOverheating == false)
			{
				IsOverheating = true;

				Overheat_BP();
			}
		}
		else if (InfiniteAmmo != 2)
		{
			AmmoInMagazine--;
		}

		BulletsShotForSmokeEffect++;

		if (GetOwningPlayer() && AmmoInMagazine <= 0 && InfiniteAmmo != 2)
		{
			if (GetOwningPlayerCast())
			{
				GetOwningPlayerCast()->Reload();
			}
		}

		FireTimerDelegate.BindUFunction(this, FName("SetCanShoot"), true);
		GetWorldTimerManager().SetTimer(FireTimerHandle, FireTimerDelegate, FireRate, false, FireRate);
	}
	else if (CurrentHeat >= MaxHeat)
	{
		StopFiring();
	}
}

void AMultiplayerGun::ServerShotgunFire_Implementation()
{
	ShotgunFire();
}

void AMultiplayerGun::ContinuousFire()
{
	if (CurrentHeat < MaxHeat || InfiniteAmmo == 2)
	{
		if (!HasAuthority())
		{
			ServerContinuousFire();
		}

		ContinuousFire_BP();

		if (IsShotgun == true)
		{
			ShotgunFire();

			GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::ShotgunFire, ContinuousFireDamageRate, true, ContinuousFireDamageRate);
		}
		else
		{
			Fire();

			GetWorldTimerManager().SetTimer(FireFullAutoTimerHandle, this, &AMultiplayerGun::Fire, ContinuousFireDamageRate, true, ContinuousFireDamageRate);
		}

		if (DoesOverheat == true && IsOverheating == false)
		{
			IsOverheating = true;

			Overheat_BP();
		}

		if (FireControllerVibration && GetOwningPlayer())
		{
			if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(GetOwningPlayer()->GetController()))
			{
				ControllerCast->VibrateController(FireControllerVibration, FireControllerVibrationTag, true);
			}
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
	if (WasPickedup == true && GetOwningPlayer())
	{
		if (GunHitEffectsReplication.HitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunHitEffectsReplication.HitEffect, GunHitEffectsReplication.HitLocation, GunHitEffectsReplication.HitRotation, HitEffectScale);
		}

		if (FireSound && FireMode != 3)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GunHitEffectsReplication.FireLocation);
		}

		if (BulletHitControllerVibration)
		{
			UGameplayStatics::SpawnForceFeedbackAtLocation(GetWorld(), BulletHitControllerVibration, GunHitEffectsReplication.HitLocation, FRotator::ZeroRotator, false, 1.0f, 0.0f, BulletHitControllerVibrationAttenuation);
		}

		if (MuzzleFlash && GetFireSceneToUse())
		{
			if (SpawnMuzzleFlashAttached == true)
			{
				UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, GetFireSceneToUse());
			}
			else
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, GetFireSceneToUse()->GetComponentLocation());
			}
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
}

void AMultiplayerGun::SpawnProjectile_Implementation(FVector FireLocation, FRotator FireRotation, FVector TraceDirection)
{
	if (ProjectileToSpawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ProjectileSpawnCollisionHandlingMethod;

		if (AMultiplayerProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AMultiplayerProjectile>(ProjectileToSpawn, FireLocation, FireRotation, SpawnParams))
		{
			SpawnedProjectile->ProjectileMesh->IgnoreActorWhenMoving(GetOwningPlayer(), true);
			SpawnedProjectile->SetOwningPlayer(GetOwningPlayer());
			SpawnedProjectile->SetLaunchPhysicsObjects(LaunchPhysicsObjects);
			SpawnedProjectile->SetLaunchObjectStrength(LaunchObjectStrength);
			SpawnedProjectile->SetLaunchObjectVelocityChange(LaunchObjectVelocityChange);
			SpawnedProjectile->SetIsExplosive(IsExplosive);
			SpawnedProjectile->SetHitEffectScale(HitEffectScale);
			SpawnedProjectile->SetExplosionIgnoredActors(ExplosionIgnoredActors);
			SpawnedProjectile->SetExplosionIgnoredClasses(ExplosionIgnoredClasses);
			SpawnedProjectile->SetTimeToDespawnProjectile(TimeToDespawnProjectile);
			SpawnedProjectile->SetDefaultDamage(DefaultDamage);
			SpawnedProjectile->SetDamage(Damage);
			SpawnedProjectile->SetExplosiveDamageRadius(ExplosiveDamageRadius);
			SpawnedProjectile->SetExplosionIgnoreOwner(ExplosionIgnoreOwner);
			SpawnedProjectile->SetBulletHitMode(BulletHitMode);
			SpawnedProjectile->SetBulletHitModeDelay(BulletHitModeDelay);
			SpawnedProjectile->SetExplosiveDoFullDamage(ExplosiveDoFullDamage);
			SpawnedProjectile->SetExplosiveCollisionChannel(CollisionChannel);
			SpawnedProjectile->SetDamageFalloffMultiplierAtRange(DamageFalloffMultiplierAtRange);
			SpawnedProjectile->SetInterpolateDamageBetweenRanges(InterpolateDamageBetweenRanges);
			SpawnedProjectile->SetPrintDistanceTraveled(PrintDistanceTraveled);
			SpawnedProjectile->SetFireLocation(FireLocation);
			SpawnedProjectile->SetHitDirection(TraceDirection);
			SpawnedProjectile->SetDefaultHitEffect(DefaultHitEffect);
			SpawnedProjectile->SetHitEffects(HitEffects);
			SpawnedProjectile->SetDefaultBulletHitSound(DefaultBulletHitSound);
			SpawnedProjectile->SetBulletHitSounds(BulletHitSounds);
			SpawnedProjectile->SetDamageType(DamageType);
			SpawnedProjectile->SetDefaultBulletHitDecal(DefaultBulletHitDecal);
			SpawnedProjectile->SetBulletHitDecals(BulletHitDecals);
			SpawnedProjectile->SetBulletHitDecalSize(BulletHitDecalSize);
			SpawnedProjectile->SetWhizzingSoundVolumeBasedOnSpeed(BulletWhizzingSoundVolumeBasedOnSpeed);
			SpawnedProjectile->SetUseActorClassesForHitMarkers(UseActorClassesForHitMarkers);
			SpawnedProjectile->SetHitMarkerActorSounds(HitMarkerActorSounds);
			SpawnedProjectile->SetHitMarkerSurfaceSounds(HitMarkerSurfaceSounds);
			SpawnedProjectile->SetCanCrumbleDestructibleMeshes(CanCrumbleDestructibleMeshes);
			SpawnedProjectile->SetDestructionSphereSize(DestructionSphereSize);
			SpawnedProjectile->SetDestructionSphereToSpawn(DestructionSphereToSpawn);
			SpawnedProjectile->SetBulletHitControllerVibration(BulletHitControllerVibration);
			SpawnedProjectile->SetBulletHitControllerVibrationAttenuation(BulletHitControllerVibrationAttenuation);
			SpawnedProjectile->SetBulletHitControllerVibrationTag(BulletHitControllerVibrationTag);
			SpawnedProjectile->FOnProjectileHit.AddDynamic(this, &AMultiplayerGun::CallBulletHitDelegate);

			if (ProjectileInheritsVelocity == true)
			{
				if (ACharacter* CharacterCast = Cast<ACharacter>(GetOwningPlayer()))
				{
					SpawnedProjectile->ProjectileMovement->Velocity += CharacterCast->GetCharacterMovement()->Velocity;
				}
			}

			if (GetOwningPlayerCast())
			{
				for (auto& Weapon : GetOwningPlayerCast()->GetAllWeapons())
				{
					if (Weapon)
					{
						SpawnedProjectile->ProjectileMesh->IgnoreActorWhenMoving(Weapon, true);
					}
				}
			}
		}
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

		if (GetBulletCasingSceneToUse())
		{
			if (AMultiplayerBulletCasing* SpawnedCasing = GetWorld()->SpawnActor<AMultiplayerBulletCasing>(BulletCasingToSpawn, GetBulletCasingSceneToUse()->GetComponentLocation(), GetBulletCasingSceneToUse()->GetComponentRotation(), SpawnParams))
			{
				SpawnedCasing->SetOwningGun(this);
				SpawnedBulletCasings.EmplaceAt(0, SpawnedCasing);

				if (BulletCasingInheritsVelocity == true && GetOwningPlayer())
				{
					if (ACharacter* CharacterCast = Cast<ACharacter>(GetOwningPlayer()))
					{
						SpawnedCasing->BulletCasingMesh->AddImpulse(CharacterCast->GetCharacterMovement()->Velocity, NAME_None, true);
					}
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "BulletCasingSceneToUse Invalid MultiplayerGun.cpp:SpawnBulletCasing");
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
	if (!HasAuthority())
	{
		ServerStopFiring();
	}

	GetWorldTimerManager().ClearTimer(FireFullAutoTimerHandle);

	if (AmountOfTimeToCancelSmoke > 0)
	{
		GetWorldTimerManager().SetTimer(CancelSmokeEffectTimerHandle, this, &AMultiplayerGun::CancelSmokeEffect, AmountOfTimeToCancelSmoke, false, AmountOfTimeToCancelSmoke);
	}
	else
	{
		BulletsShotForSmokeEffect = 0;
	}

	if (SpawnSmokeEffectWhenShooting != 0)
	{
		if (SpawnedSmokeEffect || SpawnedSmokeSound)
		{
			if (DestroySmokeEffectWhenNotShooting == true || DestroySmokeSoundWhenNotShooting == true)
			{
				if (TimeToDestroySmokeWhenNotShooting > 0)
				{
					GetWorldTimerManager().SetTimer(DestroySmokeEffectTimerHandle, this, &AMultiplayerGun::DestroySmokeEffect_BP, TimeToDestroySmokeWhenNotShooting, false, TimeToDestroySmokeWhenNotShooting);
				}
				else if (TimeToDestroySmokeSoundWhenNotShooting > 0)
				{
					GetWorldTimerManager().SetTimer(DestroySmokeEffectTimerHandle, this, &AMultiplayerGun::DestroySmokeEffect_BP, TimeToDestroySmokeSoundWhenNotShooting, false, TimeToDestroySmokeSoundWhenNotShooting);
				}
			}
		}
	}

	if (EvenCancelBurst == true)
	{
		GetWorldTimerManager().ClearTimer(BurstFireTimerHandle);

		AmountOfBurstShotsFired = 0;
	}

	if (FireMode == 3)
	{
		StopContinuousFire_BP();

		if (GetOwningPlayer() && FireControllerVibration)
		{
			if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(GetOwningPlayer()->GetController()))
			{
				ControllerCast->ClientStopForceFeedback(FireControllerVibration, FireControllerVibrationTag);
			}
		}
	}

	if (DoesOverheat == true && IsOverheating == true)
	{
		IsOverheating = false;

		CoolDown_BP();
	}
}

void AMultiplayerGun::ServerStopFiring_Implementation(bool EvenCancelBurst)
{
	StopFiring(EvenCancelBurst);
}

void AMultiplayerGun::CancelSmokeEffect()
{
	BulletsShotForSmokeEffect = 0;
}

void AMultiplayerGun::ApplyPerspective(bool ThirdPerson)
{
	SetUsingThirdPerson(ThirdPerson);

	ApplyPerspective_BP(ThirdPerson);

	if (ThirdPerson == true)
	{
		if (GetGunMesh())
		{
			GetGunMesh()->SetOwnerNoSee(true);

			if (GetThirdPersonGunMesh())
			{
				TArray<USceneComponent*> FirstPersonComponents;
				TArray<USceneComponent*> ThirdPersonComponents;

				GetGunMesh()->GetChildrenComponents(false, FirstPersonComponents);
				GetThirdPersonGunMesh()->GetChildrenComponents(false, ThirdPersonComponents);

				if (FirstPersonComponents.Num() > 0)
				{
					for (auto& Component : FirstPersonComponents)
					{
						if (Component)
						{
							if (UPrimitiveComponent* ComponentCast = Cast<UPrimitiveComponent>(Component))
							{
								ComponentCast->SetOwnerNoSee(true);
							}
						}
					}
				}

				if (ThirdPersonComponents.Num() > 0)
				{
					for (auto& Component : ThirdPersonComponents)
					{
						if (Component)
						{
							if (UPrimitiveComponent* ComponentCast = Cast<UPrimitiveComponent>(Component))
							{
								ComponentCast->SetOwnerNoSee(false);
							}
						}
					}
				}
			}
		}

		if (GetThirdPersonGunMesh())
		{
			GetThirdPersonGunMesh()->SetOwnerNoSee(false);
		}
	}
	else
	{
		if (GetGunMesh())
		{
			if (GetOwningPlayerCast())
			{
				GetGunMesh()->SetOwnerNoSee(GetOwningPlayerCast()->HideFirstPersonArmsAndGunInFirstPerson);
			}
			else
			{
				GetGunMesh()->SetOwnerNoSee(false);

				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid 1 MultiplayerGun.cpp:ApplyPerspective");
			}

			if (GetThirdPersonGunMesh())
			{
				TArray<USceneComponent*> FirstPersonComponents;
				TArray<USceneComponent*> ThirdPersonComponents;

				GetGunMesh()->GetChildrenComponents(false, FirstPersonComponents);
				GetThirdPersonGunMesh()->GetChildrenComponents(false, ThirdPersonComponents);

				if (FirstPersonComponents.Num() > 0)
				{
					for (auto& Component : FirstPersonComponents)
					{
						if (Component)
						{
							if (UPrimitiveComponent* ComponentCast = Cast<UPrimitiveComponent>(Component))
							{
								if (GetOwningPlayerCast())
								{
									ComponentCast->SetOwnerNoSee(GetOwningPlayerCast()->HideFirstPersonArmsAndGunInFirstPerson);
								}
								else
								{
									ComponentCast->SetOwnerNoSee(false);

									GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid 2 MultiplayerGun.cpp:ApplyPerspective");
								}
							}
						}
					}
				}

				if (ThirdPersonComponents.Num() > 0)
				{
					for (auto& Component : ThirdPersonComponents)
					{
						if (Component)
						{
							if (UPrimitiveComponent* ComponentCast = Cast<UPrimitiveComponent>(Component))
							{
								if (GetOwningPlayerCast())
								{
									ComponentCast->SetOwnerNoSee(GetOwningPlayerCast()->HideThirdPersonGunInFirstPerson);
								}
								else
								{
									ComponentCast->SetOwnerNoSee(true);

									GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid 3 MultiplayerGun.cpp:ApplyPerspective");
								}
							}
						}
					}
				}
			}
		}

		if (GetThirdPersonGunMesh())
		{
			if (GetOwningPlayerCast())
			{
				GetThirdPersonGunMesh()->SetOwnerNoSee(GetOwningPlayerCast()->HideThirdPersonGunInFirstPerson);
			}
			else
			{
				GetThirdPersonGunMesh()->SetOwnerNoSee(true);
				
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetOwningPlayerCast Invalid 4 MultiplayerGun.cpp:ApplyPerspective");
			}
		}
	}
}

void AMultiplayerGun::SetCanShoot(bool NewCanShoot)
{
	SwitchedFireToServer = false;
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

void AMultiplayerGun::SetSocketName(FName NewSocketName)
{
	SocketName = NewSocketName;
}

FName AMultiplayerGun::GetSocketName()
{
	return SocketName;
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

void AMultiplayerGun::SetDivideAimingFOV(bool NewDivideAimingFOV)
{
	DivideAimingFOV = NewDivideAimingFOV;
}

bool AMultiplayerGun::GetDivideAimingFOV()
{
	return DivideAimingFOV;
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

bool AMultiplayerGun::GetDoesOverheat()
{
	return DoesOverheat;
}

float AMultiplayerGun::GetCurrentHeat()
{
	return CurrentHeat;
}

float AMultiplayerGun::GetMaxHeat()
{
	return MaxHeat;
}

void AMultiplayerGun::SetUseSharedCalibers(bool NewUseSharedCalibers)
{
	UseSharedCalibers = NewUseSharedCalibers;
}

bool AMultiplayerGun::GetUseSharedCalibers()
{
	return UseSharedCalibers;
}

void AMultiplayerGun::SetCaliberToUse(int32 NewCaliberToUse)
{
	CaliberToUse = NewCaliberToUse;
}

int32 AMultiplayerGun::GetCaliberToUse()
{
	return CaliberToUse;
}

void AMultiplayerGun::SetInfiniteAmmo(int NewInfiniteAmmo)
{
	InfiniteAmmo = NewInfiniteAmmo;
}

int AMultiplayerGun::GetInfiniteAmmo()
{
	return InfiniteAmmo;
}

bool AMultiplayerGun::GetUseProjectile()
{
	return UseProjectile;
}

int AMultiplayerGun::GetSharedCaliberAmount()
{
	TArray<int32> AllCaliberAmounts;
	int CaliberAmount = 0;
	
	if (GetUseSharedCalibers() == true)
	{
		if (GetOwningPlayerCast())
		{
			GetOwningPlayerCast()->GetAllSharedCalibers().GenerateValueArray(AllCaliberAmounts);

			if (AllCaliberAmounts.IsValidIndex(CaliberToUse))
			{
				CaliberAmount = AllCaliberAmounts[CaliberToUse];
			}
			else
			{
				UseSharedCalibers = false;

				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "CaliberToUse Is Invalid MultiplayerGun.cpp:GetSharedCaliberAmount");
			}
		}
		else
		{
			UseSharedCalibers = false;

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Player Cast Failed, Using Reserve Ammo Instead MultiplayerGun.cpp:GetSharedCaliberAmount");
		}
	}

	return CaliberAmount;
}

void AMultiplayerGun::Reload()
{
	Reload_BP();

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
	ServerReload_BP();
	MulticastReload();
}

void AMultiplayerGun::MulticastReload_Implementation()
{
	if (AmmoInMagazine < MaxAmmoInMagazine)
	{
		if ((UseSharedCalibers == false && ReserveAmmo >= 1) || (UseSharedCalibers == true && GetSharedCaliberAmount() >= 1) || InfiniteAmmo == 1)
		{
			int BulletsShot = MaxAmmoInMagazine - AmmoInMagazine;

			if ((BulletsShot <= ReserveAmmo && UseSharedCalibers == false) || (BulletsShot <= GetSharedCaliberAmount() && UseSharedCalibers == true))
			{
				AmmoInMagazine = MaxAmmoInMagazine;

				if (InfiniteAmmo == 0)
				{
					if (UseSharedCalibers == true)
					{
						GetOwningPlayerCast()->SetSharedCaliberAmount(CaliberToUse, GetOwningPlayerCast()->GetSharedCaliberAmount(CaliberToUse) - BulletsShot);
					}
					else
					{
						ReserveAmmo -= BulletsShot;
					}
				}

				BulletsShot = 0;
			}
			else if ((BulletsShot > ReserveAmmo && UseSharedCalibers == false) || (BulletsShot > GetSharedCaliberAmount() && UseSharedCalibers == true))
			{
				if (InfiniteAmmo == 0)
				{
					if (UseSharedCalibers == true)
					{
						AmmoInMagazine += GetOwningPlayerCast()->GetSharedCaliberAmount(CaliberToUse);
						GetOwningPlayerCast()->SetSharedCaliberAmount(CaliberToUse, 0);
					}
					else
					{
						AmmoInMagazine += ReserveAmmo;
						ReserveAmmo = 0;
					}
				}
				else
				{
					AmmoInMagazine = MaxAmmoInMagazine;
				}

				BulletsShot = 0;
			}

			if (ReloadGunSound)
			{
				UPrimitiveComponent* GunMesh = GetGunMesh();
				UPrimitiveComponent* ThirdPersonGunMesh = GetThirdPersonGunMesh();

				if (GunMesh)
				{
					UGameplayStatics::SpawnSoundAttached(ReloadGunSound, GunMesh, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);
				}
				else if (ThirdPersonGunMesh)
				{
					UGameplayStatics::SpawnSoundAttached(ReloadGunSound, ThirdPersonGunMesh, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GunMesh and ThirdPersonGunMesh Invalid MultiplayerGun.cpp:MulticastReload");
				}
			}
		}
	}
}

void AMultiplayerGun::SetIsExplosive(bool NewIsExplosive)
{
	IsExplosive = NewIsExplosive;
}

bool AMultiplayerGun::GetIsExplosive()
{
	return IsExplosive;
}

void AMultiplayerGun::SetUseActorClassesForHitMarkers(int NewUseActorClassesForHitMarkers)
{
	UseActorClassesForHitMarkers = NewUseActorClassesForHitMarkers;
}

int AMultiplayerGun::GetUseActorClassesForHitMarkers()
{
	return UseActorClassesForHitMarkers;
}

void AMultiplayerGun::SetHitMarkerActorSounds(TMap<TSubclassOf<AActor>, USoundBase*> NewHitMarkerActorSounds)
{
	HitMarkerActorSounds = NewHitMarkerActorSounds;
}

TMap<TSubclassOf<AActor>, USoundBase*> AMultiplayerGun::GetHitMarkerActorSounds()
{
	return HitMarkerActorSounds;
}

void AMultiplayerGun::SetHitMarkerSurfaceSounds(TMap<UPhysicalMaterial*, USoundBase*> NewHitMarkerSurfaceSounds)
{
	HitMarkerSurfaceSounds = NewHitMarkerSurfaceSounds;
}

TMap<UPhysicalMaterial*, USoundBase*> AMultiplayerGun::GetHitMarkerSurfaceSounds()
{
	return HitMarkerSurfaceSounds;
}

void AMultiplayerGun::SetReloadGunSound(USoundBase* NewReloadGunSound)
{
	ReloadGunSound = NewReloadGunSound;
}

USoundBase* AMultiplayerGun::GetReloadGunSound()
{
	return ReloadGunSound;
}

// Called when the game starts or when spawned
void AMultiplayerGun::BeginPlay()
{
	Super::BeginPlay();

	if (WasPickedupBeginPlay == false)
	{
		SetWasPickedup(false);
	}

	AddPredeterminedSpread();

	GetWorldTimerManager().SetTimer(CheckForOwnerTimerHandle, this, &AMultiplayerGun::CheckForOwner, 0.5f, true, 0.5f);
}

// Called every frame
void AMultiplayerGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FireMode == 0 || FireMode == 2)
	{
		DoesOverheat = false;
	}

	if (IsShotgun == true)
	{
		UseProjectile = false;
	}

	if (IsExplosive == true)
	{
		UseActorClassesForHitMarkers = 0;
	}

	if (DoesOverheat == false && SpawnSmokeEffectWhenShooting == 2)
	{
		SpawnSmokeEffectWhenShooting = 1;
	}

	if (UseFirstPersonRotationForThirdPersonMuzzleFlash == true)
	{
		ThirdPersonFireSceneComponent->SetWorldRotation(FireSceneComponent->GetComponentRotation());
	}
}

void AMultiplayerGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerGun, OwningPlayer);
	DOREPLIFETIME(AMultiplayerGun, OwningPlayerCast);
	DOREPLIFETIME(AMultiplayerGun, UsingThirdPerson);
	DOREPLIFETIME(AMultiplayerGun, WasPickedup);
	DOREPLIFETIME(AMultiplayerGun, AmmoInMagazine);
	DOREPLIFETIME(AMultiplayerGun, ReserveAmmo);
	DOREPLIFETIME(AMultiplayerGun, InfiniteAmmo);
	DOREPLIFETIME(AMultiplayerGun, ShotgunAmountOfPelletsShot);
	DOREPLIFETIME(AMultiplayerGun, PreDeterminedSpreadIndex);
	DOREPLIFETIME(AMultiplayerGun, PreDeterminedAimingSpreadIndex);
	DOREPLIFETIME(AMultiplayerGun, PreDeterminedAimingSpreadWithMultiplierIndex);
	DOREPLIFETIME(AMultiplayerGun, ShotgunPelletHitLocations);
	DOREPLIFETIME_CONDITION(AMultiplayerGun, GunHitEffectsReplication, COND_SkipOwner);
}