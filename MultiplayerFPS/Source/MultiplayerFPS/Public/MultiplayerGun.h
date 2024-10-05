// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableItem.h"
#include "MultiplayerProjectile.h"
#include "MultiplayerBulletCasing.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "MultiplayerGun.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletHit, AActor*, ActorHit, UPhysicalMaterial*, HitSurface);

class USceneComponent;
class UAnimMontage;
class UParticleSystem;
class USoundBase;
class UMaterialInterface;
class UCameraShakeBase;
class UDamageType;
class UForceFeedbackAttenuation;
class AMultiplayerCharacter;

USTRUCT()
struct FGunHitEffectsReplication
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector FireLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator HitRotation = FRotator::ZeroRotator;

	UPROPERTY()
	UParticleSystem* HitEffect = nullptr;

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

	UPROPERTY(BlueprintAssignable, Category = "Functions")
	FOnBulletHit OnBulletHit;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	UPrimitiveComponent* GetGunMesh();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	UPrimitiveComponent* GetThirdPersonGunMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to the owning player"))
	UStaticMeshComponent* GunStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to the owning player"))
	USkeletalMeshComponent* GunSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to other players"))
	UStaticMeshComponent* ThirdPersonGunStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is only visible to other players"))
	USkeletalMeshComponent* ThirdPersonGunSkeletalMesh;

	// Set this to false in the default constructor to use a static mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This needs to be changed in C++"))
	bool UseSkeletalMesh;

	UFUNCTION(BlueprintCallable, meta = (Tooltip = "This will make sure the owner is valid so if for example a player disconnects their gun won't just be floating where they last were"), Category = "Functions")
	virtual void CheckForOwner();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions", meta = (Tooltip = "This can return nullptr so check that it's valid when using, requires using MultiplayerCharacter class"))
	virtual AMultiplayerCharacter* GetOwningPlayerCast();
	
	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual USceneComponent* GetFireSceneToUse();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual USceneComponent* GetBulletCasingSceneToUse();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUsingThirdPerson(bool NewUsingThirdPerson);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerSetUsingThirdPerson(bool NewUsingThirdPerson);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetUsingThirdPerson(bool NewUsingThirdPerson);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetUsingThirdPerson();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetWasPickedup(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SetWasPickedup_BP(bool Pickedup, UPrimitiveComponent* ComponentToAttachTo = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetWasPickedup();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetPickupCollisionEnabled();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSetPickupCollisionEnabled();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetPickupCollisionEnabled();

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "For ReplicationMethod 0 = Not replicated, 1 = Server only, 2 = Multicast only, 3 = Both server and multicast, 4 = Server if client and multicast if host", ClampMin = 0, ClampMax = 4))
	virtual void SetOwningPlayer(APawn* NewOwningPlayer, int ReplicationMethod = 0);

	UFUNCTION(Server, Reliable, Category = "Functions", meta = (ClampMin = 0, ClampMax = 3))
	virtual void ServerSetOwningPlayer(APawn* NewOwningPlayer, int ReplicationMethod = 0);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetOwningPlayer(APawn* NewOwningPlayer);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	APawn* GetOwningPlayer();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void FireInput();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ChargeUp();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ChargeUp1();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ChargeUp_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void CancelChargeUp();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void CancelChargeUp1();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void CancelChargeUp_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void Fire();

	UFUNCTION(Server, Reliable, Category = "Functions")
	void ServerFire();

	UFUNCTION(Client, Reliable, Category = "Functions")
	void ClientFire();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void Fire_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ServerFire_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ClientFire_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void AddPredeterminedSpread();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Functions")
	void ExecuteHitFunction(AActor* ParentPlayer = nullptr, AActor* HitActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void CallBulletHitDelegate(AActor* HitActor, UPhysicalMaterial* HitSurface);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ContinuousFire_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void StopContinuousFire_BP();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void SpawnProjectile(FVector FireLocation, FRotator FireRotation, FVector TraceDirection);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ShotgunFire();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerShotgunFire();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ContinuousFire();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerContinuousFire();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void Overheat_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void CoolDown_BP();

	UFUNCTION(Client, Reliable, Category = "Functions")
	virtual void SpawnBulletCasing();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void CheckBulletCasingLimit();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void StopFiring(bool EvenCancelBurst = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerStopFiring(bool EvenCancelBurst = false);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void AddRecoil_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SpawnSmokeEffect_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void CancelSmokeEffect();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void DestroySmokeEffect_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "Do not call this function, this is used in the character SetUsingThirdPerson funtion"))
	virtual void ApplyPerspective(bool ThirdPerson);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ApplyPerspective_BP(bool ThirdPerson);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCanShoot(bool NewCanShoot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetCanShoot();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetFireMode(int NewFireMode);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetSocketName(FName NewSocketName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FName GetSocketName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetPlayerArmsRelativeLocation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FRotator GetPlayerArmsRelativeRotation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetFireMode();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUseADS(int NewUseADS);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetUseADS();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetDivideAimingFOV(bool NewDivideAimingFOV);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetDivideAimingFOV();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetADSFOV(float NewADSFOV);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetADSFOV();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetZoomFOV(float NewZoomFOV);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetZoomFOV();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetTimeToADS(float NewTimeToADS);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetTimeToADS();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetTimeToZoom(float NewTimeToZoom);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetTimeToZoom();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetADSArmsLocation(FVector NewADSArmsLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetADSArmsLocation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetADSArmsRotation(FRotator NewADSArmsRotation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FRotator GetADSArmsRotation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetZoomArmsLocation(FVector NewZoomArmsLocation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FVector GetZoomArmsLocation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetZoomArmsRotation(FRotator NewZoomArmsRotation);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FRotator GetZoomArmsRotation();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAmmoInMagazine(int NewAmmoInMagazine);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetAmmoInMagazine();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetMaxAmmoInMagazine();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetReserveAmmo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetMaxReserveAmmo();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUseSharedCalibers(bool NewUseSharedCalibers);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetUseSharedCalibers();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCaliberToUse(int32 NewCaliberToUse);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int32 GetCaliberToUse();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHasChargeUp(bool NewHasChargeup);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetHasChargeUp();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetHoldTriggerDuringChargeUp(bool NewHoldTriggerDuringChargeUp);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetHoldTriggerDuringChargeUp();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetChargeUpTime(float NewChargeUpTime);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetChargeUpTime();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCurrentChargeUpProgress(float NewCurrentChargeUpProgress);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetCurrentChargeUpProgress();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SetChargeUpProgressTimeForTimeline(float NewTime);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetChargeUpTimeRemaining();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UAnimMontage* GetChargeUpArmsAnimationMontage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UAnimationAsset* GetChargeUpArmsAnimation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetDoesOverheat();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetCurrentHeat();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetMaxHeat();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetInfiniteAmmo(int NewInfiniteAmmo);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetInfiniteAmmo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetUseProjectile();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetSharedCaliberAmount();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void Reload();

	UFUNCTION(Server, Reliable, Category = "Functions")
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	void MulticastReload();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void Reload_BP();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ServerReload_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetIsExplosive(bool NewIsExplosive);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetIsExplosive();

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
	virtual void SetReloadGunSound(USoundBase* NewReloadGunSound);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual USoundBase* GetReloadGunSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Name")
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (Tooltip = "Only set this variable if you are placing the gun in the level, do not set this at runtime"))
	bool WasPickedupBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "The amount of time it takes to refill the magazine, 0 = instant, -1 = time for player animation to finish, -2 = time for gun animation to finish, if you have animation montages assigned time will be based on the animation montage", ClampMin = -2))
	float ReloadSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "The amount of time it takes to finish reloading after the magazine was refilled, 0 = instant, -1 = time for player animation to finish, -2 = time for gun animation to finish, if you have animation montages assigned time will be based on the animation montage", ClampMin = -2))
	float ReloadSpeed1;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = Semi-Auto, 1 = Full-Auto, 2 = Burst, 3 = Continuous, continuous fire would be for something like a flamethrower that is constantly firing so for example sound isn't played when damage is applied it's looped until you stop firing, if this is true it is recommended to set UseProjectile to false and BulletCasingToSpawn to none", ClampMin = 0, ClampMax = 3))
	int FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This would be useful for something like a flamethrower that has an area of effect rather than having the player hit only what is in the center of the screen, does not apply if UseProjectile = true"))
	bool UseBoxCollisionForDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Does not work with projectiles"))
	bool IsShotgun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 1))
	int ShotgunAmountOfPellets;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	int ShotgunAmountOfPelletsShot;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	TArray<FVector> ShotgunPelletHitLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = no, 1 = fire location is at FireSceneComponent, 2 = fire rotation is based on FireSceneComponent, 3 = fire location and rotation is at FireSceneComponent", ClampMin = 0, ClampMax = 3))
	int FireFromBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "X axis is to the left and right, Y is forward and backword, and Z is up and down"))
	FVector BulletSpawnLocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Overrides aiming spread variables"))
	bool UseAimingSpreadMultiplier;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MinAimingFireVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MaxAimingFireVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MinAimingFireHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	float MaxAimingFireHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Spread is pre determined so it can replicate properly"))
	bool AutomaticallyAddPreDeterminedSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "If AutomaticallyAddPreDeterminedSpread = true 15 values will automatically be added, if you want to add your own you can manually do it here, setting this manually will override the min and max spread variables, only set the X and Y axes Z or 'yaw' does nothing"))
	TArray<FRotator> PreDeterminedSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "If AutomaticallyAddPreDeterminedSpread = true 15 values will automatically be added, if you want to add your own you can manually do it here, setting this manually will override the min and max spread variables, only set the X and Y axes Z or 'yaw' does nothing"))
	TArray<FRotator> PreDeterminedAimingSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "If AutomaticallyAddPreDeterminedSpread = true 15 values will automatically be added, if you want to add your own you can manually do it here, setting this manually will override the min and max spread variables, only set the X and Y axes Z or 'yaw' does nothing"))
	TArray<FRotator> PreDeterminedAimingSpreadWithMultiplier;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	int32 PreDeterminedSpreadIndex;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	int32 PreDeterminedAimingSpreadIndex;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firing")
	int32 PreDeterminedAimingSpreadWithMultiplierIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup", meta = (Tooltip = "If true there will be a chargeup before the gun can fire"))
	bool HasChargeUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup", meta = (Tooltip = "If true then releasing the trigger during the chargeup will cancel firing"))
	bool HoldTriggerDuringChargeUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup", meta = (Tooltip = "The amount of time it takes to chargeup", ClampMin = 0.0f))
	float ChargeUpTime;

	UPROPERTY(BlueprintReadWrite, Category = "Chargeup")
	float CurrentChargeUpProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup")
	UParticleSystem* ChargeUpParticleEffect;

	UPROPERTY(BlueprintReadWrite, Category = "Chargeup")
	UParticleSystemComponent* SpawnedChargeUpParticleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup", meta = (Tooltip = "If this is true then the muzzle flash will move with the gun instead of staying in the same spot"))
	bool SpawnChargeUpParticleAttached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chargeup")
	bool DestroyChargeUpParticleWhenChargeUpCanceled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheating", meta = (Tooltip = "Only applies if FireMode = 1 or 3, if this is true firing will make it overheat instead of using ammo"))
	bool DoesOverheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheating", meta = (Tooltip = "0 = can fire while cooling down, 1 = can't fire while cooling down if max heat is reached, 2 = can't fire while cooling down no matter what", ClampMin = 0, ClampMax = 2))
	int ProhibitFiringWhileCoolingDown;

	UPROPERTY(BlueprintReadWrite, Category = "Overheating")
	bool ReachedMaxHeat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheating", meta = (ClampMin = 0.001f))
	float TimeToOverheat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheating", meta = (ClampMin = 0.001f))
	float TimeToCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overheating", meta = (ClampMin = 0.0f))
	float MaxHeat;

	UPROPERTY(BlueprintReadWrite, Category = "Overheating", meta = (ClampMin = 0.0f))
	float CurrentHeat;

	UPROPERTY(BlueprintReadWrite, Category = "Overheating")
	bool IsOverheating;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "If set to 0 it will just use the animation length, if this is not 0 it will use this to determine how long it takes to switch off of and onto this weapon, if you have animation montages assigned time will be based on the animation montage", ClampMin = 0.0f))
	float WeaponSwitchTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimationAsset* WeaponSwitchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimMontage* WeaponSwitchAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimationAsset* ThirdPersonWeaponSwitchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimMontage* ThirdPersonWeaponSwitchAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	bool UseTwoWeaponSwitchAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimationAsset* WeaponSwitchAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimMontage* WeaponSwitchAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimationAsset* ThirdPersonWeaponSwitchAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimMontage* ThirdPersonWeaponSwitchAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Played when switching to this gun, only applies if you are using a skeletal mesh for your gun"))
	UAnimationAsset* SwitchToGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Played when switching to this gun, only applies if you are using a skeletal mesh for your gun"))
	UAnimMontage* SwitchToGunAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Played when switching to another gun, only applies if you are using a skeletal mesh for your gun"))
	UAnimationAsset* SwitchOffGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Played when switching to another gun, only applies if you are using a skeletal mesh for your gun"))
	UAnimMontage* SwitchOffGunAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms"))
	UAnimMontage* ReloadAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms when the gun is emptied"))
	UAnimationAsset* ReloadEmptyAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms when the gun is emptied"))
	UAnimMontage* ReloadEmptyAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component"))
	UAnimationAsset* ThirdPersonReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component"))
	UAnimMontage* ThirdPersonReloadAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component when the gun is emptied"))
	UAnimationAsset* ThirdPersonReloadEmptyAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component when the gun is emptied"))
	UAnimMontage* ThirdPersonReloadEmptyAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	bool UseTwoReloadAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms if UseTwoReloadAnimations = true"))
	UAnimationAsset* ReloadAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms if UseTwoReloadAnimations = true"))
	UAnimMontage* ReloadAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms when the gun is emptied if UseTwoReloadAnimations = true"))
	UAnimationAsset* ReloadEmptyAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's arms when the gun is emptied if UseTwoReloadAnimations = true"))
	UAnimMontage* ReloadEmptyAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	bool UseTwoThirdPersonReloadAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component if UseTwoReloadAnimations = true"))
	UAnimationAsset* ThirdPersonReloadAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component if UseTwoReloadAnimations = true"))
	UAnimMontage* ThirdPersonReloadAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component when the gun is emptied if UseTwoReloadAnimations = true"))
	UAnimationAsset* ThirdPersonReloadEmptyAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "This plays on the character's mesh component when the gun is emptied if UseTwoReloadAnimations = true"))
	UAnimMontage* ThirdPersonReloadEmptyAnimation1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Only applies if you are using a skeletal mesh for your gun"))
	UAnimationAsset* ReloadGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Only applies if you are using a skeletal mesh for your gun"))
	UAnimMontage* ReloadGunAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Plays when the gun is emptied, only applies if you are using a skeletal mesh for your gun"))
	UAnimationAsset* ReloadEmptyGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Plays when the gun is emptied, only applies if you are using a skeletal mesh for your gun"))
	UAnimMontage* ReloadEmptyGunAnimationMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	TArray<AMultiplayerBulletCasing*> SpawnedBulletCasings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is where the muzzle flash will be"))
	USceneComponent* FireSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This is where the muzzle flash will be for other players"))
	USceneComponent* ThirdPersonFireSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* BulletCasingSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* ThirdPersonBulletCasingSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This only matters if UseBoxCollisionForDamage = true, rescale this instead of the box collision"))
	USceneComponent* FireBoxScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "This only matters if UseBoxCollisionForDamage = true"))
	UBoxComponent* DamageBoxCollision;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	APawn* OwningPlayer;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	AMultiplayerCharacter* OwningPlayerCast;

	// This is only used to control fire rate, to control if the player can fire set can shoot in the player character class
	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	bool CanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo", meta = (ClampMin = 0))
	int AmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	int MaxAmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo", meta = (ClampMin = 0))
	int ReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (ClampMin = 0))
	int MaxReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "Whether to use reserve ammo specific to this gun or use calibers shared by other guns that are chambered in the same caliber"))
	bool UseSharedCalibers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (Tooltip = "This will use the index of the AllSharedCalibers variable on the player character starting at 0, only applies if UseSharedCalibers = true", ClampMin = 0))
	int32 CaliberToUse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo", meta = (Tooltip = "0 = limited ammo, 1 = infinite reserve ammo, 2 = infinite ammo, this will override DoesOverheat", ClampMin = 0, ClampMax = 2))
	int InfiniteAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	TSubclassOf<AMultiplayerBulletCasing> BulletCasingToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	ESpawnActorCollisionHandlingMethod BulletCasingSpawnCollisionHandlingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	bool BulletCasingInheritsVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int MaxAmountOfBulletCasings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "The amount of time between shots"), meta = (ClampMin = 0.001f))
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "The amount of time between applying damage for continuous fire"), meta = (ClampMin = 0.0f))
	float ContinuousFireDamageRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 0))
	int AmountOfShotsForBurst;

	UPROPERTY(BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 0))
	int AmountOfBurstShotsFired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if UseProjectile = false", ClampMin = 0.0f))
	float FireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "For this to replicate you need to replicate movement for the actor you're launching, for explosives this only applies to projectiles"))
	bool LaunchPhysicsObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if LaunchPhysicsObjects = true", ClampMin = 0.001f))
	float LaunchObjectStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if LaunchPhysicsObjects = true"))
	bool LaunchObjectVelocityChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	bool IsExplosive;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	ESpawnActorCollisionHandlingMethod ProjectileSpawnCollisionHandlingMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool ProjectileInheritsVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ToolTip = "This will determine the amount of time before projectile despawn if they don't hit anything, for explosives this will detonate them, 0 = disabled, only applies if UseProjectile = true", ClampMin = 0.0f))
	float TimeToDespawnProjectile;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "This will also be the collision channel for explosives"))
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "The owner and the gun are already ignored, if you want to ignore more actors set this at runtime, only applies if UseProjectile = false"))
	TArray<AActor*> AdditionalActorsToIgnore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Damage applied to everything that has either no physical material or one not included in damage variable, for explosives this will serve as the base damage"), meta = (ClampMin = 0))
	float DefaultDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Add pysical materials like the head to apply different damage, if left blank it will just apply default damage"))
	TMap<UPhysicalMaterial*, float> Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0.0f))
	float ExplosiveDamageRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool ExplosiveDoFullDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "0 = just apply damage, 1 = apply damage and execute ExecuteHitFunction(), 2 = just execute ExecuteHitFunction(), to use this override the ExecuteHitFunction() or add event ExecuteHitFunction, for projectiles you will need to define this function in the projectile, this function only runs on server", ClampMin = 0, ClampMax = 2))
	int BulletHitMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "The first float is the distance traveled and the second float is the damage multiplier at that range, values above 1 do more damage, does not apply to explosives"))
	TMap<float, float> DamageFalloffMultiplierAtRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Only applies if DamageFalloffMultiplierAtRange has 2 or more values, if the target distance is in between 2 distances in the DamageFalloffMultiplierAtRange variable the damage will also be in between the 2 damages in the DamageFalloffMultiplierAtRange variable"))
	bool InterpolateDamageBetweenRanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "This will print a string showing how far the bullet went"))
	bool PrintDistanceTraveled;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void PrintDistanceTraved_BP(float Distance);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	bool UsingThirdPerson;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	bool WasPickedup;
	
	UPROPERTY()
	int AmountOfTimesPickedup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (Tooltip = "Set to 0 to disable", ClampMin = 0.0f))
	float TimeToDespawnAfterDropped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Setting this to 0 will make the gun not use a socket and instead just use relative location and rotation, setting this to 1 will snap to socket without scale, and 2 will snap to socket including scale"), meta = (ClampMin = 0, ClampMax = 2))
	int SnapToSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "If SnapToSocket = 0 then it will just attach to a socket if you set this variable"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "If SnapToSocket = 0 then it will just attach to a socket if you set this variable"))
	FName ThirdPersonSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands)"))
	FVector GunRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands)"))
	FRotator GunRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands), usually you can set this to the same as for first person"))
	FVector ThirdPersonGunRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location", meta = (Tooltip = "Only applies if SnapToSocket = 0 and is relative to its parent component (usually the player character's hands), usually you can set this to the same as for first person"))
	FRotator ThirdPersonGunRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location")
	FVector PlayerArmsRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Location")
	FRotator PlayerArmsRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "This will also serve as the explosion for explosives"))
	UParticleSystem* DefaultHitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "For explosives this will also be the explosion scale"))
	FVector HitEffectScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Add pysical materials like the head to spawn different particle effects, if left blank it will just use the default hit effect"))
	TMap<UPhysicalMaterial*, UParticleSystem*> HitEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "If this is true then the muzzle flash will move with the gun instead of staying in the same spot"))
	bool SpawnMuzzleFlashAttached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "If this is false other players will see the muzzle flash at the ThirdPersonFireSceneComponent, if this is true other players will see the muzzle flash in the same location as the player firing, this also applies to the charge up particle if you have the gun charge up before firing"))
	bool ReplicateMuzzleFlashLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This also applies to the charge up particle if you have the gun charge up before firing"))
	bool UseFirstPersonRotationForThirdPersonMuzzleFlash;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	UParticleSystemComponent* SpawnedContinuousMuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "0 = no, 1 = yes, 2 = indicator for overheating if DoesOverheat = true, will be set to 1 if DoesOverheat = false", ClampMin = 0, ClampMax = 2))
	int SpawnSmokeEffectWhenShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "only applies if SpawnSmokeEffectWhenShooting = 1", ClampMin = 0))
	int AmountOfShotsToSpawnSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Amount of time to not shoot to cancel spawning smoke", ClampMin = 0.001f))
	float AmountOfTimeToCancelSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This would be used if you have a particle effect that loops and will not auto destroy"))
	bool DestroySmokeEffectWhenNotShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This would be used if you have a sound effect that loops and will not auto destroy"))
	bool DestroySmokeSoundWhenNotShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Amount of time to not shoot after spawning the smoke to destroy it, set to 0 to disable, this would be used if you have a particle effect that loops and will not auto destroy, overrides TimeToDestroySmokeSoundWhenNotShooting", ClampMin = 0.001))
	float TimeToDestroySmokeWhenNotShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (ClampMin = 0.001))
	float TimeToDestroySmokeSoundWhenNotShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Between 0 and 1, only applies if SpawnSmokeEffectWhenShooting = 2", ClampMin = 0.001, ClampMax = 1))
	float PercentageOfOverheatToSpawnSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	UParticleSystem* SmokeEffectToSpawn;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	UParticleSystemComponent* SpawnedSmokeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if SpawnSmokeEffectWhenShooting is not 0, clear to not have a sound"))
	USoundBase* SmokeSoundToSpawn;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	UAudioComponent* SpawnedSmokeSound;

	UPROPERTY(BlueprintReadWrite, Category = "Firing")
	int BulletsShotForSmokeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Only applies if BulletHitMode equals 1 or 2, and works better for projectiles, set to 0 to disable", ClampMin = 0.0f))
	float BulletHitModeDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Clear this to disable"))
	UForceFeedbackEffect* FireControllerVibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "Used to cancel vibration with continuous fire"))
	FName FireControllerVibrationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing", meta = (Tooltip = "This also applies to explosives, clear this to disable"))
	UForceFeedbackEffect* BulletHitControllerVibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	UForceFeedbackAttenuation* BulletHitControllerVibrationAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing")
	FName BulletHitControllerVibrationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "If this is true then the fire sound will move with the gun instead of staying in the same spot"))
	bool SpawnFireSoundAttached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "This will only affect the player firing the gun, this is to avoid the sound being louder in one ear"))
	bool SpawnFireSound2DForOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "This will only affect the player firing the gun, this is to avoid the sound being louder in one ear"))
	bool SpawnFireSound2DForOwnerThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* FireSoundAttenuationOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* ThirdPersonFireSoundAttenuationOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ChargeUpSound;

	UPROPERTY(BlueprintReadWrite, Category = "Sound")
	UAudioComponent* SpawnedChargeUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "If this is true then the fire sound will move with the gun instead of staying in the same spot"))
	bool SpawnChargeUpSoundAttached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "This will only affect the player firing the gun, this is to avoid the sound being louder in one ear"))
	bool SpawnChargeUpSound2DForOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "This will only affect the player firing the gun, this is to avoid the sound being louder in one ear"))
	bool SpawnChargeUpSound2DForOwnerThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "Does not apply if SpawnChargeUpSoundAttached = false"))
	bool DestroyChargeUpSoundWhenChargeUpCanceled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* ChargeUpSoundAttenuationOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* ThirdPersonChargeUpSoundAttenuationOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ReloadGunSound;

	UPROPERTY(BlueprintReadWrite, Category = "Sound")
	UAudioComponent* SpawnedContinuousFireSound;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "0 = hit actor classes, 1 = hit physical materials, 2 = prioritize physical material but fallback on actor class, only 0 works for explosives and box collision damage", ClampMin = 0, ClampMax = 2))
	int UseActorClassesForHitMarkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Only applies if UseActorClassesForHitMarkers = true"))
	TMap<TSubclassOf<AActor>, USoundBase*> HitMarkerActorSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects", meta = (Tooltip = "Using the physical material allows you to have a different sound for each surface, only applies if UseActorClassesForHitMarkers = false"))
	TMap<UPhysicalMaterial*, USoundBase*> HitMarkerSurfaceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "If set to 0 it will play the animation montage, if set to 1 it will play the animation, if set to 2 it will play both", ClampMin = 0, ClampMax = 2))
	int UseFireArmsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* FireArmsAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* FireArmsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Only applies if you are using a skeletal mesh for your gun"))
	UAnimationAsset* FireGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* ChargeUpArmsAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* ChargeUpArmsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* ChargeUpGunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "0 = will use ADS if player character allows, 1 = will use zoom if player character allows, 2 = will use ADS overriding variable in player character, 3 = will use zoom overriding variable in player character", ClampMin = 0, ClampMax = 3))
	int UseADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "If false aiming will subtract from current FOV, if true aiming will divide from current FOV"))
	bool DivideAimingFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "This is how much to subtract or divide from current FOV"))
	float ADSFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (Tooltip = "This is how much to subtract or divide from current FOV"))
	float ZoomFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (ClampMin = 0.001f))
	float TimeToADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (ClampMin = 0.001f))
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction", meta = (Tooltip = "Can cause stutters when firing especially on the client"))
	bool CanCrumbleDestructibleMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction", meta = (Tooltip = "Only applies if IsShotgun = true, having this equal false will crumble destructibles at the average hit location of each pellet, setting this to true can cause stutters when firing especially on the client"))
	bool CrumbleDestructibleMeshesWithEveryShotgunPellet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos Destruction", meta = (Tooltip = "Also set this for explosives, destruction sphere will use this not damage radius"))
	FVector DestructionSphereSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chaos Destruction", meta = (Tooltip = "This is used to destroy destructible meshes, make this either FS_MasterField or a child of it"))
	TSubclassOf<AActor> DestructionSphereToSpawn;

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

	UPROPERTY()
	bool SwitchedFireToServer;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle CheckForOwnerTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle DespawnTimerHandle;
	
	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle ChargeUpTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle CancelChargeUpTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle SpawnProjectileTimerHandle;

	FTimerDelegate SpawnProjectileTimerDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle FireTimerHandle;

	FTimerDelegate FireTimerDelegate;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle FireFullAutoTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle BurstFireTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle CancelSmokeEffectTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle DestroySmokeEffectTimerHandle;

	UPROPERTY(BlueprintReadWrite, Category = "Timers")
	FTimerHandle BulletHitModeTimerHandle;

	FTimerDelegate BulletHitModeTimerDelegate;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};