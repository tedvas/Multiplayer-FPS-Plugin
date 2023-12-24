// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MultiplayerProjectile.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiplayerProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAudioComponent* BulletWhizzingSoundComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION(BlueprintCallable)
	virtual void OnProjectileMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void RegisterHit(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Explode();

	UFUNCTION(BlueprintCallable)
	void SpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MulticastSpawnHitParticleEffect(UParticleSystem* ParticleEffect, FVector ImpactPoint, FRotator ImpactRotation, FHitResult Hit);

	UFUNCTION(BlueprintCallable)
	virtual void SetOwningPlayer(APawn* NewOwningPlayer);

	UFUNCTION(BlueprintCallable)
	virtual APawn* GetOwningPlayer();

	UFUNCTION(BlueprintCallable)
	virtual void SetIsExplosive(bool NewIsExplosive);

	UFUNCTION(BlueprintCallable)
	virtual bool GetIsExplosive();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosionScale(FVector NewExplosionScale);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetExplosionScale();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosionIgnoredActors(TArray<AActor*> NewExplosionIgnoredActors);

	UFUNCTION(BlueprintCallable)
	virtual TArray<AActor*> GetExplosionIgnoredActors();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosionIgnoredClasses(TArray<TSubclassOf<AActor>> NewExplosionIgnoredClasses);

	UFUNCTION(BlueprintCallable)
	virtual TArray<TSubclassOf<AActor>> GetExplosionIgnoredClasses();

	UFUNCTION(BlueprintCallable)
	virtual void SetDefaultDamage(float NewDefaultDamage);

	UFUNCTION(BlueprintCallable)
	virtual float GetDefaultDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetHeadDamage(float NewHeadDamage);

	UFUNCTION(BlueprintCallable)
	virtual float GetHeadDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetTorsoDamage(float NewTorsoDamage);

	UFUNCTION(BlueprintCallable)
	virtual float GetTorsoDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetLegDamage(float NewLegDamage);

	UFUNCTION(BlueprintCallable)
	virtual float GetLegDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosionIgnoreOwner(bool NewExplosionIgnoreOwner);

	UFUNCTION(BlueprintCallable)
	virtual bool GetExplosionIgnoreOwner();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosiveDamageRadius(float NewExplosiveDamageRadius);

	UFUNCTION(BlueprintCallable)
	virtual float GetExplosiveDamageRadius();

	UFUNCTION(BlueprintCallable)
	virtual void SetExplosiveDoFullDamage(bool NewExplosiveDoFullDamage);

	UFUNCTION(BlueprintCallable)
	virtual bool GetExplosiveDoFullDamage();

	UFUNCTION(BlueprintCallable)
	virtual void SetHitDirection(FVector NewHitDirection);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetHitDirection();

	UFUNCTION(BlueprintCallable)
	virtual void SetBloodEffect(UParticleSystem* NewBloodEffect);

	UFUNCTION(BlueprintCallable)
	virtual UParticleSystem* GetBloodEffect();

	UFUNCTION(BlueprintCallable)
	virtual void SetDefaultHitEffect(UParticleSystem* NewDefaultHitEffect);

	UFUNCTION(BlueprintCallable)
	virtual UParticleSystem* GetDefaultHitEffect();

	UFUNCTION(BlueprintCallable)
	virtual void SetDefaultBulletHitSound(USoundBase* NewDefaultBulletHitSound);

	UFUNCTION(BlueprintCallable)
	virtual USoundBase* GetDefaultBulletHitSound();

	UFUNCTION(BlueprintCallable)
	virtual void SetBulletHitSounds(TMap<USoundBase*, UPhysicalMaterial*> NewBulletHitSounds);

	UFUNCTION(BlueprintCallable)
	virtual TMap<USoundBase*, UPhysicalMaterial*> GetBulletHitSounds();

	UFUNCTION(BlueprintCallable)
	virtual void SetDamageType(TSubclassOf<UDamageType> NewDamageType);

	UFUNCTION(BlueprintCallable)
	virtual TSubclassOf<UDamageType> GetDamageType();

	UFUNCTION(BlueprintCallable)
	virtual void SetDefaultBulletHitDecal(UMaterialInterface* NewBulletHitDecal);

	UFUNCTION(BlueprintCallable)
	virtual UMaterialInterface* GetDefaultBulletHitDecal();

	UFUNCTION(BlueprintCallable)
	virtual void SetBulletHitDecals(TMap<UMaterialInterface*, UPhysicalMaterial*> NewBulletHitDecals);

	UFUNCTION(BlueprintCallable)
	virtual TMap<UMaterialInterface*, UPhysicalMaterial*> GetBulletHitDecals();

	UFUNCTION(BlueprintCallable)
	virtual void SetBulletHitDecalSize(FVector NewBulletHitDecalSize);

	UFUNCTION(BlueprintCallable)
	virtual FVector GetBulletHitDecalSize();

	UFUNCTION(BlueprintCallable)
	virtual void SetWhizzingSoundVolumeBasedOnSpeed(bool NewWhizzingSoundVolumeBasedOnSpeed);

	UFUNCTION(BlueprintCallable)
	virtual bool GetWhizzingSoundVolumeBasedOnSpeed();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated)
	APawn* OwningPlayer;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Damage")
	bool IsExplosive;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Damage")
	FVector ExplosionScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	bool ExplosionIgnoreOwner;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TArray<AActor*> ExplosionIgnoredActors;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TArray<TSubclassOf<AActor>> ExplosionIgnoredClasses;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float DefaultDamage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float HeadDamage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float TorsoDamage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float LegDamage;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float ExplosiveDamageRadius;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	bool ExplosiveDoFullDamage;

	UPROPERTY(BlueprintReadWrite)
	FVector HitDirection;

	UPROPERTY(BlueprintReadWrite, Category = "Particles")
	UParticleSystem* Blood;

	UPROPERTY(BlueprintReadWrite, Category = "Particles")
	UParticleSystem* DefaultHitEffect;

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

	float BulletWhizzingSoundVolume;

	UPROPERTY(Replicated)
	bool RegisteredHit;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};