// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "MultiplayerGun.h"
#include "MultiplayerHealthComponent.h"
#include "MultiplayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputMappingContext* IMC_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	int IMC_Priority;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_GamepadLook;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Interact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Aim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_Reload;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_SwitchWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_GamepadSwitchWeapons;

public:
	// Sets default values for this character's properties
	AMultiplayerCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual UMultiplayerHealthComponent* GetHealthComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* FPCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* ArmsMesh;

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void SetupInput();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void RemoveInput();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void GetOwningController();

	virtual void RecalculateBaseEyeHeight() override;
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void GamepadLook(const FInputActionValue& Value);
	virtual void PressJump();
	virtual void HoldJump();

	UFUNCTION(BlueprintCallable)
	virtual void SetSensitivity();

	UFUNCTION(BlueprintCallable)
	virtual void Interact();

	UFUNCTION(Server, Reliable)
	virtual void ServerInteract();

	UFUNCTION(BlueprintCallable)
	virtual void PickupItem(AInteractableItem* ItemToPickup);

	UFUNCTION(Server, Reliable)
	virtual void ServerPickupItem(AInteractableItem* ItemToPickup);

	UFUNCTION(BlueprintCallable)
	virtual void SetCanInteract(bool NewCanInteract);

	UFUNCTION(Server, Reliable)
	virtual void ServerSetCanInteract(bool NewCanInteract);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSetCanInteract(bool NewCanInteract);

	UFUNCTION(BlueprintCallable)
	virtual bool GetCanInteract();

	UFUNCTION(BlueprintCallable)
	virtual void SetInteractDistance(float NewInteractDistance);

	UFUNCTION(BlueprintCallable)
	virtual float GetInteractDistance();

	UFUNCTION(BlueprintCallable)
	virtual void SetOverlappingWeapon(bool NewOverlappingWeapon);

	UFUNCTION(BlueprintCallable)
	virtual bool GetOverlappingWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponBeingOverlapped(AMultiplayerGun* NewWeaponBeingOverlapped);

	UFUNCTION(BlueprintCallable)
	virtual AMultiplayerGun* GetWeaponBeingOverlapped();

	UFUNCTION(BlueprintCallable)
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	virtual void Die1();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void ServerDie1();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void ClientDie();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable)
	virtual void SetCanShoot(bool NewCanShoot);

	UFUNCTION(Server, Reliable)
	virtual void ServerSetCanShoot(bool NewCanShoot);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSetCanShoot(bool NewCanShoot);

	UFUNCTION(BlueprintCallable)
	virtual bool GetCanShoot();

	UPROPERTY(BlueprintReadWrite, Replicated)
	int CurrentWeaponIndex;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<AMultiplayerGun*> GetAllWeapons();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual AMultiplayerGun* GetWeapon(bool GetCurrentWeapon = true, int WeaponIndex = -1);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual int GetWeaponIndex(AMultiplayerGun* Weapon);

	UFUNCTION(BlueprintCallable)
	virtual int GetAmountOfWeapons();

	UFUNCTION(BlueprintCallable)
	virtual int GetMaxWeaponAmount();

	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponVisibility(int WeaponVisibilityToChange, bool Visible, bool SetAllOtherWeaponsToOppositeVisibility = true);

	UFUNCTION(BlueprintCallable)
	virtual void GiveLoadout(TArray<TSubclassOf<AMultiplayerGun>> Loadout, int MaxWeaponAmount = 2);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void GiveWeapon(TSubclassOf<AMultiplayerGun> WeaponToSpawn, AMultiplayerGun* WeaponToPickup = nullptr, bool SwitchToNewWeapon = false);

	UFUNCTION(BlueprintCallable)
	virtual void AddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(Server, Reliable)
	virtual void ServerAddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastAddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(BlueprintCallable)
	virtual void RemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(Server, Reliable)
	virtual void ServerRemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastRemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeaponsInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(Server, Reliable)
	virtual void ServerSwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeapons1();

	UFUNCTION(Server, Reliable)
	virtual void ServerSwitchWeapons1();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSwitchWeapons1();

	UFUNCTION(BlueprintCallable)
	virtual void NextWeapon();

	UFUNCTION(Server, Reliable)
	virtual void ServerNextWeapon();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastNextWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void LastWeapon();

	UFUNCTION(Server, Reliable)
	virtual void ServerLastWeapon();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastLastWeapon();

	UFUNCTION(BlueprintCallable)
	virtual void PressFireInput();

	UFUNCTION(BlueprintCallable)
	virtual void ReleaseFireInput();

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	UFUNCTION(BlueprintCallable)
	virtual void StopFiring(bool EvenCancelBurst = false);

	UFUNCTION(BlueprintCallable)
	virtual void AimInput();

	UFUNCTION(BlueprintCallable)
	virtual void ReleaseAimInput();

	UFUNCTION(BlueprintCallable)
	virtual void Aim();

	UFUNCTION(BlueprintCallable)
	virtual void StopAiming();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetAimingFOV_BP(bool Aiming, float AimingFOV, float TimeToAim);

	UFUNCTION(BlueprintCallable)
	virtual void SetCanAim(bool NewCanAim);

	UFUNCTION(BlueprintCallable)
	virtual bool GetCanAim();

	UFUNCTION(BlueprintCallable)
	virtual void SetIsAiming(bool NewIsAiming);

	UFUNCTION(BlueprintCallable)
	virtual bool GetIsAiming();

	UFUNCTION(BlueprintCallable)
	virtual void SetIsADSing(bool NewIsADSing);

	UFUNCTION(BlueprintCallable)
	virtual bool GetIsADSing();

	UFUNCTION(BlueprintCallable)
	virtual void SetIsZoomedIn(bool NewIsZoomedIn);

	UFUNCTION(BlueprintCallable)
	virtual bool GetIsZoomedIn();

	UFUNCTION(BlueprintCallable)
	virtual void SetUseADS(int NewUseADS);

	UFUNCTION(BlueprintCallable)
	virtual int GetUseADS();

	UFUNCTION(BlueprintCallable)
	virtual void Reload();

	UFUNCTION(Server, Reliable)
	virtual void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastReload();

	UFUNCTION(BlueprintCallable)
	void Reload1();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerReload1();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MulticastReload1();

	UFUNCTION(BlueprintCallable)
	void Reload2();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerReload2();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MulticastReload2();

	UFUNCTION(BlueprintCallable)
	virtual void CancelReload(bool PutArmsBackUp = true);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerCancelReload(bool PutArmsBackUp = true);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void MulticastCancelReload(bool PutArmsBackUp = true);

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	int PlayerIndex;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	APlayerController* OwningController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMultiplayerHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interact")
	bool CanInteract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float InteractDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	bool OverlappingWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	AMultiplayerGun* WeaponBeingOverlapped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons")
	bool CanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool ShootingCancelsReload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This will determine whether weapons are dropped or destroyed on death"))
	bool DropWeaponsOnDeath;

	UPROPERTY(BlueprintReadWrite)
	bool HoldingFireInput;

	UPROPERTY(BlueprintReadWrite)
	bool HoldingAimInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons")
	bool CanAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool AimingCancelsReload;

	UPROPERTY(BlueprintReadWrite)
	bool IsAiming;

	UPROPERTY(BlueprintReadWrite)
	bool IsADSing;

	UPROPERTY(BlueprintReadWrite)
	bool IsZoomedIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (Tooltip = "0 = determined by weapon, 1 = ADS with all weapons, 2 = zoom in with all weapons", ClampMin = 0, ClampMax = 2))
	int UseADS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (ClampMin = 0))
	float TimeToSwitchWeapons;

	UPROPERTY(BlueprintReadWrite)
	bool IsSwitchingWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FieldOfView;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float CurrentFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the X axis when not aiming"))
	float MouseDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float MouseAimingSensitivityX;

	UPROPERTY(BlueprintReadWrite)
	float CurrentMouseSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the Y axis when not aiming"))
	float MouseDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float MouseAimingSensitivityY;

	UPROPERTY(BlueprintReadWrite)
	float CurrentMouseSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MouseAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the X axis when not aiming"))
	float GamepadDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float GamepadAimingSensitivityX;

	UPROPERTY(BlueprintReadWrite)
	float CurrentGamepadSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the Y axis when not aiming"))
	float GamepadDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float GamepadAimingSensitivityY;

	UPROPERTY(BlueprintReadWrite)
	float CurrentGamepadSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float GamepadAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool UseAimSensitivityMultipler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool ToggleAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool HoldButtonToJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FName HeadSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FVector CameraHeadLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FRotator CameraHeadRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (ClampMin = 0))
	float RespawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (ClampMin = 0))
	float DelayToDestroyBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	bool UseTwoWeaponSwitchAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanReload;

	UPROPERTY(BlueprintReadWrite)
	bool IsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* WeaponSwitchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* WeaponSwitchAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimationAsset* ReloadAnimation1;

	UPROPERTY(BlueprintReadWrite, Replicated)
	TArray<AMultiplayerGun*> AllWeapons;

	FTimerHandle DestroySelfTimerHandle;
	FTimerDelegate SwitchWeaponsTimerDelegate;
	FTimerHandle SwitchWeaponsTimerHandle;
	FTimerHandle ReloadTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};