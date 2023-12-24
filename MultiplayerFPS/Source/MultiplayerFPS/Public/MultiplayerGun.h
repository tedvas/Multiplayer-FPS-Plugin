// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableItem.h"
#include "MultiplayerProjectile.h"
#include "MultiplayerBulletCasing.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MultiplayerGun.generated.h"

class USceneComponent;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class UMaterialInterface;
class UCameraShakeBase;
class UDamageType;

USTRUCT()
struct FGunHitEffectsReplication
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector FireLocation;

	UPROPERTY()
	FVector HitLocation;

	UPROPERTY()
	FRotator HitRotation;

	UPROPERTY()
	UParticleSystem* HitEffect;

	UPROPERTY()
	FHitResult HitResult;
};

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerGun : public AInteractableItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiplayerGun();

	UFUNCTION(BlueprintCallable, meta = (Tooltip = "This will make sure the owner is valid so if for example a player disconnects their gun won't just be floating where they last were"))
	virtual void CheckForOwner();

	UFUNCTION(BlueprintCallable)
	virtual void OnPickupBoxCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	virtual void OnPickupBoxCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	virtual void SetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(Server, Reliable)
	virtual void ServerSetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual void SetPickupCollisionEnabled();

	UFUNCTION(Server, Reliable)
	virtual void ServerSetPickupCollisionEnabled();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSetPickupCollisionEnabled();

	UFUNCTION(BlueprintCallable)
	virtual void SetOwningPlayer(APawn* NewOwningPlayer);

	UFUNCTION(BlueprintCallable)
	APawn* GetOwningPlayer();

	UFUNCTION(BlueprintCallable)
	virtual void FireInput();

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	UFUNCTION(Server, Reliable)
	virtual void ServerFire();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void SpawnProjectile(FVector FireLocation, FRotator FireRotation, FVector TraceDirection);

	UFUNCTION(BlueprintCallable)
	virtual void ShotgunFire();

	UFUNCTION(Server, Reliable)
	virtual void ServerShotgunFire();

	UFUNCTION(BlueprintCallable)
	virtual void ContinuousFire();

	UFUNCTION(Server, Reliable)
	virtual void ServerContinuousFire();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Overheat_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void CoolDown_BP();

	UFUNCTION(Client, Reliable)
	virtual void SpawnBulletCasing();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void CheckBulletCasingLimit();

	UFUNCTION(BlueprintCallable)
	virtual void StopFiring(bool EvenCancelBurst = false);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddRecoil_BP();

	UFUNCTION(BlueprintCallable)
	virtual void SetCanShoot(bool NewCanShoot);

	UFUNCTION(BlueprintCallable)
	virtual bool GetCanShoot();

	UFUNCTION(BlueprintCallable)
	virtual void SetFireMode(int NewFireMode);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetPlayerArmsRelativeLocation();

	UFUNCTION(BlueprintCallable)
	virtual FRotator GetPlayerArmsRelativeRotation();

	UFUNCTION(BlueprintCallable)
	virtual int GetFireMode();

	UFUNCTION(BlueprintCallable)
	virtual void SetUseADS(int NewUseADS);

	UFUNCTION(BlueprintCallable)
	virtual int GetUseADS();

	UFUNCTION(BlueprintCallable)
	virtual void SetADSFOV(float NewADSFOV);

	UFUNCTION(BlueprintCallable)
	virtual float GetADSFOV();

	UFUNCTION(BlueprintCallable)
	virtual void SetZoomFOV(float NewZoomFOV);

	UFUNCTION(BlueprintCallable)
	virtual float GetZoomFOV();

	UFUNCTION(BlueprintCallable)
	virtual void SetTimeToADS(float NewTimeToADS);

	UFUNCTION(BlueprintCallable)
	virtual float GetTimeToADS();

	UFUNCTION(BlueprintCallable)
	virtual void SetTimeToZoom(float NewTimeToZoom);

	UFUNCTION(BlueprintCallable)
	virtual float GetTimeToZoom();

	UFUNCTION(BlueprintCallable)
	virtual void SetADSArmsLocation(FVector NewADSArmsLocation);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetADSArmsLocation();

	UFUNCTION(BlueprintCallable)
	virtual void SetADSArmsRotation(FRotator NewADSArmsRotation);

	UFUNCTION(BlueprintCallable)
	virtual FRotator GetADSArmsRotation();

	UFUNCTION(BlueprintCallable)
	virtual void SetZoomArmsLocation(FVector NewZoomArmsLocation);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetZoomArmsLocation();

	UFUNCTION(BlueprintCallable)
	virtual void SetZoomArmsRotation(FRotator NewZoomArmsRotation);

	UFUNCTION(BlueprintCallable)
	virtual FRotator GetZoomArmsRotation();

	UFUNCTION(BlueprintCallable)
	virtual void SetAmmoInMagazine(int NewAmmoInMagazine);

	UFUNCTION(BlueprintCallable)
	virtual int GetAmmoInMagazine();

	UFUNCTION(BlueprintCallable)
	virtual int GetMaxAmmoInMagazine();

	UFUNCTION(BlueprintCallable)
	virtual int GetReserveAmmo();

	UFUNCTION(BlueprintCallable)
	virtual int GetMaxReserveAmmo();

	UFUNCTION(BlueprintCallable)
	virtual bool GetUseProjectile();

	UFUNCTION(BlueprintCallable)
	virtual void Reload();

	UFUNCTION(Server, Reliable)
	virtual void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastReload();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "The amount of time it takes to refill the magazine", ClampMin = 0))
	float ReloadSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "The amount of time it takes to finish reloading after the magazine was refilled", ClampMin = 0))
	float ReloadSpeed1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Name")
	FName WeaponName;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	TArray<AMultiplayerBulletCasing*> SpawnedBulletCasings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to the owning player"))
	UStaticMeshComponent* GunStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to the owning player"))
	USkeletalMeshComponent* GunSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to other players"))
	UStaticMeshComponent* ThirdPersonGunStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to other players"))
	USkeletalMeshComponent* ThirdPersonGunSkeletalMesh;

	// Set this to false in the default constructor to use a static mesh
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	bool UseSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is where the muzzle flash will be and if you have fire location and or rotation based on the barrel it will be based on this component"))
	USceneComponent* FireSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* BulletCasingSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* PickupBoxCollision;

	UPROPERTY(BlueprintReadWrite, Replicated)
	APawn* OwningPlayer;

	// This is only used to control fire rate, to control if the player can fire set can shoot in the player character class
	UPROPERTY(BlueprintReadWrite)
	bool CanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = Semi-Auto, 1 = Full-Auto, 2 = Burst, 3 = Continuous, continuous fire would be for something like a flamethrower that is constantly firing so for example sound isn't played when damage is applied it's looped until you stop firing and is not recommended if UseProjectile = true", ClampMin = 0, ClampMax = 3))
	int FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Does not work with projectiles"))
	bool IsShotgun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	int ShotgunAmountOfPellets;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	int ShotgunAmountOfPelletsShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float ShotgunMinVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float ShotgunMaxVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float ShotgunMinHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float ShotgunMaxHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = will not have spread, 1 = will only have spread when hip firing, 2 = will have spread when hip firing and zoomed in but not when ADSing, 3 = will have spread when hip firing and ADSing but not when zoom, 4 = will have spread no matter what", ClampMin = 0, ClampMax = 4))
	int HaveBulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float SpreadAimingMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MinHipFireVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MaxHipFireVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MinHipFireHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MaxHipFireHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo", meta = (ClampMin = 0))
	int AmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	int MaxAmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo", meta = (ClampMin = 0))
	int ReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	int MaxReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "Only applies if FireMode = 1 or 3, if this is true firing will make it overheat instead of using ammo"))
	bool DoesOverheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	float TimeToOverheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	float MaxHeat;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	float CurrentHeat;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	bool IsOverheating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	TSubclassOf<AMultiplayerBulletCasing> BulletCasingToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	ESpawnActorCollisionHandlingMethod BulletCasingSpawnCollisionHandlingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	bool BulletCasingInheritsVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int MaxAmountOfBulletCasings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "The amount of time between shots"), meta = (ClampMin = 0))
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "The amount of time between applying damage for continuous fire"), meta = (ClampMin = 0))
	float ContinuousFireDamageRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 0))
	int AmountOfShotsForBurst;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 0))
	int AmountOfBurstShotsFired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if UseProjectile = false"))
	float FireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	bool IsExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	FVector ExplosionScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	bool ExplosionIgnoreOwner;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Set this at runtime rather than setting the default value"))
	TArray<AActor*> ExplosionIgnoredActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	TArray<TSubclassOf<AActor>> ExplosionIgnoredClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (Tooltip = "Does not work with shotguns"))
	bool UseProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AMultiplayerProjectile> ProjectileToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (Tooltip = "Raising this number will spawn the projectile further forward to avoid colliding with the owner"))
	float ProjectileSpawnForwardOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	ESpawnActorCollisionHandlingMethod ProjectileSpawnCollisionHandlingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool ProjectileInheritsVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Damage applied to the chest and everything else, for explosives this will serve as the base damage"), meta = (ClampMin = 0))
	float DefaultDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0))
	float HeadDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0))
	float TorsoDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0))
	float LegDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0))
	float ExplosiveDamageRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool ExplosiveDoFullDamage;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool WasPickedup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Setting this to 0 will make the gun not use a socket and instead just use relative location and rotation, setting this to 1 will snap to socket without scale, and 2 will snap to socket including scale"), meta = (ClampMin = 0, ClampMax = 2))
	int SnapToSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "If SnapToSocket = 0 then it will just attach to a socket if you set this variable"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands)"))
	FVector GunRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands)"))
	FRotator GunRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location")
	FVector PlayerArmsRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location")
	FRotator PlayerArmsRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects")
	UParticleSystem* Blood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "This will also serve as the explosion for explosives"))
	UParticleSystem* DefaultHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (Tooltip = "Only applies to projectiles"))
	bool BulletWhizzingSoundVolumeBasedOnSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "This will also serve as the explosion sound for explosives"))
	USoundBase* DefaultBulletHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Using the physical material allows you to have a different sound for each surface"))
	TMap<USoundBase*, UPhysicalMaterial*> BulletHitSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects")
	UMaterialInterface* DefaultBulletHitDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Using the physical material allows you to have a different decal for each surface"))
	TMap<UMaterialInterface*, UPhysicalMaterial*> BulletHitDecals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects")
	FVector BulletHitDecalSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* FireArmsAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "0 = will use ADS if player character allows, 1 = will use zoom if player character allows, 2 = will use ADS overriding variable in player character, 3 = will use zoom overriding variable in player character", ClampMin = 0, ClampMax = 3))
	int UseADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "This is how much to subtract from current FOV"))
	float ADSFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "This is how much to subtract from current FOV"))
	float ZoomFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float TimeToADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float TimeToZoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FVector ADSArmsLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FRotator ADSArmsRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FVector ZoomArmsLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	FRotator ZoomArmsRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float VerticalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float HorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category = "Debugging", meta = (Tooltip = "Only applies if UseProjectile = false, if true a debug line will be drawn when firing to show where the line trace goes"))
	bool ShowBulletPath;

	UPROPERTY(EditAnywhere, Category = "Debugging")
	FColor BulletPathColor;

	UPROPERTY(EditAnywhere, Category = "Debugging")
	bool BulletPathPersists;

	UPROPERTY(EditAnywhere, Category = "Debugging")
	float BulletPathDuration;

	UPROPERTY(ReplicatedUsing = OnRep_GunHitEffects)
	FGunHitEffectsReplication GunHitEffectsReplication;

	UFUNCTION()
	void OnRep_GunHitEffects();

	UAudioComponent* SpawnedContinuousFireSound;
	UParticleSystemComponent* SpawnedContinuousMuzzleFlash;

	FTimerHandle CheckForOwnerTimerHandle;
	FTimerDelegate FireTimerDelegate;
	FTimerDelegate SpawnProjectileTimerDelegate;
	FTimerHandle SpawnProjectileTimerHandle;
	FTimerHandle FireTimerHandle;
	FTimerHandle FireFullAutoTimerHandle;
	FTimerHandle BurstFireTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};