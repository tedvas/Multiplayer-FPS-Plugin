// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "TimerManager.h"
#include "MultiplayerProjectile.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHit, AActor*, ActorHit, UPhysicalMaterial*, HitSurface);

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiplayerProjectile();

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnProjectileHit FOnProjectileHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* BulletWhizzingSoundComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URadialForceComponent* RadialForceComponent;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnProjectileMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void RegisterHit(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void Explode(const FHitResult& Hit, bool UseCurrentLocationForHit = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void DetermineBeginPlayDespawnTimer();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void DetermineBulletHitModeDelay();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Functions")
	void ExecuteHitFunction(AActor* ParentPlayer = nullptr, AActor* HitActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit = false);

	UFUNCTION(Server, Reliable, Category = "Functions")
	void ServerSpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit = false);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	void MulticastSpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit, bool UseCurrentLocationForHit = false);

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "This will hide the projectile if you need it to not be actually destroyed but still hidden"))
	virtual void SoftDestroyProjectile();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSoftDestroyProjectile();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSoftDestroyProjectile();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SilenceBulletWhizzingSound();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSilenceBulletWhizzingSound();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSilenceBulletWhizzingSound();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetOwningPlayer(APawn* NewOwningPlayer);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual APawn* GetOwningPlayer();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetLaunchPhysicsObjects(bool NewLaunchPhysicsObjects);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetLaunchPhysicsObjects();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetLaunchObjectStrength(float NewLaunchObjectStrength);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetLaunchObjectStrength();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetLaunchObjectVelocityChange(bool NewLaunchObjectVelocityChange);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetLaunchObjectVelocityChange();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetIsExplosive(bool NewIsExplosive);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetIsExplosive();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHitEffectScale(FVector NewHitEffectScale);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetHitEffectScale();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosionIgnoredActors(TArray<AActor*> NewExplosionIgnoredActors);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TArray<AActor*> GetExplosionIgnoredActors();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosionIgnoredClasses(TArray<TSubclassOf<AActor>> NewExplosionIgnoredClasses);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TArray<TSubclassOf<AActor>> GetExplosionIgnoredClasses();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetTimeToDespawnProjectile(float NewTimeToDespawnProjectile);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetTimeToDespawnProjectile();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDefaultDamage(float NewDefaultDamage);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetDefaultDamage();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDamage(TMap<UPhysicalMaterial*, float> NewDamage);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<UPhysicalMaterial*, float> GetDamage();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosionIgnoreOwner(bool NewExplosionIgnoreOwner);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetExplosionIgnoreOwner();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitMode(int NewBulletHitMode);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetBulletHitMode();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitModeDelay(float NewBulletHitModeDelay);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetBulletHitModeDelay();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosiveDamageRadius(float NewExplosiveDamageRadius);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetExplosiveDamageRadius();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosiveDoFullDamage(bool NewExplosiveDoFullDamage);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetExplosiveDoFullDamage();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetExplosiveCollisionChannel(TEnumAsByte<ECollisionChannel> NewExplosiveCollisionChannel);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TEnumAsByte<ECollisionChannel> GetExplosiveCollisionChannel();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDamageFalloffMultiplierAtRange(TMap<float, float> NewDamageFalloffMultiplierAtRange);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<float, float> GetDamageFalloffMultiplierAtRange();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetInterpolateDamageBetweenRanges(bool NewInterpolateDamageBetweenRanges);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetInterpolateDamageBetweenRanges();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetPrintDistanceTraveled(bool NewPrintDistanceTraveled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetPrintDistanceTraveled();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetFireLocation(FVector NewFireLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetFireLocation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHitDirection(FVector NewHitDirection);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetHitDirection();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDefaultHitEffect(UParticleSystem* NewDefaultHitEffect);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UParticleSystem* GetDefaultHitEffect();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHitEffects(TMap<UPhysicalMaterial*, UParticleSystem*> NewHitEffects);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<UPhysicalMaterial*, UParticleSystem*> GetHitEffects();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDefaultBulletHitSound(USoundBase* NewDefaultBulletHitSound);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual USoundBase* GetDefaultBulletHitSound();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitSounds(TMap<USoundBase*, UPhysicalMaterial*> NewBulletHitSounds);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<USoundBase*, UPhysicalMaterial*> GetBulletHitSounds();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDamageType(TSubclassOf<UDamageType> NewDamageType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TSubclassOf<UDamageType> GetDamageType();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDefaultBulletHitDecal(UMaterialInterface* NewBulletHitDecal);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UMaterialInterface* GetDefaultBulletHitDecal();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitDecals(TMap<UMaterialInterface*, UPhysicalMaterial*> NewBulletHitDecals);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<UMaterialInterface*, UPhysicalMaterial*> GetBulletHitDecals();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitDecalSize(FVector NewBulletHitDecalSize);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetBulletHitDecalSize();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetWhizzingSoundVolumeBasedOnSpeed(bool NewWhizzingSoundVolumeBasedOnSpeed);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetWhizzingSoundVolumeBasedOnSpeed();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUseActorClassesForHitMarkers(int NewUseActorClassesForHitMarkers);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetUseActorClassesForHitMarkers();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHitMarkerActorSounds(TMap<TSubclassOf<AActor>, USoundBase*> NewHitMarkerActorSounds);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<TSubclassOf<AActor>, USoundBase*> GetHitMarkerActorSounds();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHitMarkerSurfaceSounds(TMap<UPhysicalMaterial*, USoundBase*> NewHitMarkerSurfaceSounds);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<UPhysicalMaterial*, USoundBase*> GetHitMarkerSurfaceSounds();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCanCrumbleDestructibleMeshes(bool NewCanCrumbleDestructibleMeshes);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetCanCrumbleDestructibleMeshes();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDestructionSphereSize(FVector NewDestructionSphereSize);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetDestructionSphereSize();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDestructionSphereToSpawn(TSubclassOf<AActor> NewDestructionSphereToSpawn);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TSubclassOf<AActor> GetDestructionSphereToSpawn();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitControllerVibration(UForceFeedbackEffect* NewBulletHitControllerVibration);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UForceFeedbackEffect* GetBulletHitControllerVibration();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitControllerVibrationAttenuation(UForceFeedbackAttenuation* NewBulletHitControllerVibrationAttenuation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UForceFeedbackAttenuation* GetBulletHitControllerVibrationAttenuation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetBulletHitControllerVibrationTag(FName NewBulletHitControllerVibrationTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FName GetBulletHitControllerVibrationTag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	APawn* OwningPlayer;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "For this to replicate you need to replicate movement for the actor you're launching"))
	bool LaunchPhysicsObjects;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if LaunchPhysicsObjects = true", ClampMin = 0.001f))
	float LaunchObjectStrength;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if LaunchPhysicsObjects = true"))
	bool LaunchObjectVelocityChange;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Damage")
	bool IsExplosive;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Damage")
	FVector HitEffectScale;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	bool ExplosionIgnoreOwner;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = just apply damage, 1 = apply damage and execute ExecuteHitFunction(), 2 = just execute ExecuteHitFunction(), to use this override the ExecuteHitFunction() or add event ExecuteHitFunction, for projectiles you will need to define this function in the projectile, this function only runs on server", ClampMin = 0, ClampMax = 2))
	int BulletHitMode;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if BulletHitMode equals 1 or 2, and works better for projectiles, set to 0 to disable", ClampMin = 0.0f))
	float BulletHitModeDelay;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TArray<AActor*> ExplosionIgnoredActors;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TArray<TSubclassOf<AActor>> ExplosionIgnoredClasses;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile", meta = (ToolTip = "This will determine the amount of time before explosive projectiles explode if they don't hit anything, 0 = disabled, only applies if UseProjectile and IsExplosive are true", ClampMin = 0.0f))
	float TimeToDespawnProjectile;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float DefaultDamage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Add pysical materials like the head to apply different damage, if left blank it will just apply default damage"))
	TMap<UPhysicalMaterial*, float> Damage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float ExplosiveDamageRadius;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	bool ExplosiveDoFullDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	TEnumAsByte<ECollisionChannel> ExplosiveCollisionChannel;

	UPROPERTY(BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "The first float is the distance traveled and the second float is the damage multiplier at that range, values above 1 do more damage, does not apply to explosives"))
	TMap<float, float> DamageFalloffMultiplierAtRange;

	UPROPERTY(BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Only applies if DamageFalloffMultiplierAtRange has 2 or more values, if the target distance is in between 2 distances in the DamageFalloffMultiplierAtRange variable the damage will also be in between the 2 damages in the DamageFalloffMultiplierAtRange variable"))
	bool InterpolateDamageBetweenRanges;

	UPROPERTY(BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "This will print a string showing how far the bullet went"))
	bool PrintDistanceTraveled;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	FVector FireLocation;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void PrintDistanceTraved_BP(float Distance);

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	FVector HitDirection;

	UPROPERTY(BlueprintReadWrite, Category = "Particles")
	UParticleSystem* DefaultHitEffect;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Add pysical materials like the head to spawn different particle effects, if left blank it will just use the default hit effect"))
	TMap<UPhysicalMaterial*, UParticleSystem*> HitEffects;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Hit Effects")
	USoundBase* DefaultBulletHitSound;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	TMap<USoundBase*, UPhysicalMaterial*> BulletHitSounds;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	UMaterialInterface* DefaultBulletHitDecal;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Using the physical material allows you to have a different decal for each surface"))
	TMap<UMaterialInterface*, UPhysicalMaterial*> BulletHitDecals;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	FVector BulletHitDecalSize;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(BlueprintReadOnly, Category = "Sound")
	bool WhizzingSoundVolumeBasedOnSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Sound")
	float BulletWhizzingSoundVolume;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "0 = hit actor classes, 1 = hit physical materials, 2 = prioritize physical material but fallback on actor class, does not apply to explosives", ClampMin = 0, ClampMax = 2))
	int UseActorClassesForHitMarkers;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Only applies if UseActorClassesForHitMarkers = true"))
	TMap<TSubclassOf<AActor>, USoundBase*> HitMarkerActorSounds;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Using the physical material allows you to have a different sound for each surface, only applies if UseActorClassesForHitMarkers = false"))
	TMap<UPhysicalMaterial*, USoundBase*> HitMarkerSurfaceSounds;

	UPROPERTY(BlueprintReadWrite, Category = "Chaos Destruction")
	bool CanCrumbleDestructibleMeshes;

	UPROPERTY(BlueprintReadWrite, Category = "Chaos Destruction", meta = (Tooltip = "Also set this for explosives, destruction sphere will use this not damage radius"))
	FVector DestructionSphereSize;

	UPROPERTY(BlueprintReadOnly, Category = "Chaos Destruction", meta = (Tooltip = "This is used to destroy destructible meshes, make this either FS_MasterField or a child of it"))
	TSubclassOf<AActor> DestructionSphereToSpawn;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This also applies to explosives, clear this to disable"))
	UForceFeedbackEffect* BulletHitControllerVibration;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	UForceFeedbackAttenuation* BulletHitControllerVibrationAttenuation;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	FName BulletHitControllerVibrationTag;

	UPROPERTY(Replicated)
	bool RegisteredHit;

	FTimerDelegate BulletHitModeTimerDelegate;
	FTimerHandle BulletHitModeTimerHandle;
	FTimerDelegate BeginPlayExplosionTimerDelegate;
	FTimerHandle BeginPlayDespawnTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};