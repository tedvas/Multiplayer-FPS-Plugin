// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MultiplayerGun.h"
#include "MultiplayerHealthComponent.h"
#include "MultiplayerPlayerController.h"
#include "MultiplayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraShakeBase;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_SwitchPerspective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_SwitchToWeapon1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Input")
	UInputAction* IA_SwitchToWeapon2;

public:
	// Sets default values for this character's properties
	AMultiplayerCharacter();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ApplySettings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UMultiplayerHealthComponent* GetHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (Tooltip = "To change this either change it in C++ or in the construction script"))
	USceneComponent* SpringArmParent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* ArmsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* FirstPersonPlayerModel;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Functions", meta = (Tooltip = "Override this function to set a different mesh"))
	USkeletalMeshComponent* GetPlayerModelMesh();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void SetupInput();

	UFUNCTION(Client, Reliable, Category = "Functions")
	virtual void ClientSetupInput();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void RemoveInput();

	UFUNCTION(Client, Reliable, Category = "Functions")
	virtual void ClientRemoveInput();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions", meta = (Tooltip = "This will automatically get the owning controller, so it does not need an input"))
	virtual void SetOwningController();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	AMultiplayerPlayerController* GetOwningController();

	UFUNCTION()
	virtual void PrintStringForOwningControllerInvalid();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ReplicateCameraTransform(FVector CameraLocation, FRotator CameraRotation);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerReplicateCameraTransform(FVector CameraLocation, FRotator CameraRotation);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastReplicateCameraTransform(FVector CameraLocation, FRotator CameraRotation);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerReplicateControlRotation(FRotator ControlRotation);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastReplicateControlRotation(FRotator ControlRotation);

	virtual void RecalculateBaseEyeHeight() override;
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void GamepadLook(const FInputActionValue& Value);
	virtual void PressJump();
	virtual void HoldJump();
	virtual void ReleaseJump();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetSensitivity();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SetFOV_BP(float NewFOV);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Interact();

	UFUNCTION(Category = "Functions")
	virtual void InteractReplicated(AInteractableItem* Interactable);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerInteract(AInteractableItem* Interactable, bool Multicast);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastInteract(AInteractableItem* Interactable);

	UFUNCTION(Client, Reliable, Category = "Functions")
	virtual void ClientInteract(AInteractableItem* Interactable);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void PickupItem(AInteractableItem* ItemToPickup);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerPickupItem(AInteractableItem* ItemToPickup);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastPickupItem(AInteractableItem* ItemToPickup);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUsingThirdPerson(bool NewUsingThirdPerson, bool SnapCameraLocation = false);

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void ClientSetUsingThirdPerson(bool NewUsingThirdPerson, bool SnapCameraLocation = false);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerSetUsingThirdPerson(bool NewUsingThirdPerson);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetUsingThirdPerson(bool NewUsingThirdPerson);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ApplyPerspectiveVisibility();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void ClientApplyPerspectiveVisibility();

	UPROPERTY()
	bool AppliedPerspectiveVisibilityOnClient = false;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ToggleThirdPerson();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SwitchPerspective_BP(bool NewUsingThirdPerson, bool SnapCameraLocation = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetUsingThirdPerson();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCanInteract(bool NewCanInteract);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSetCanInteract(bool NewCanInteract);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetCanInteract(bool NewCanInteract);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetCanInteract();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetInteractDistance(float NewInteractDistance);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual float GetInteractDistance();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetOverlappingInteractable(bool NewOverlappingInteractable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetOverlappingInteractable();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetInteractableBeingOverlapped(AInteractableItem* NewInteractableBeingOverlapped);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual AInteractableItem* GetInteractableBeingOverlapped();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnTakeDamage(int OldHealth, int NewHealth);

	UFUNCTION(Client, Reliable, Category = "Functions")
	virtual void ClientOnTakeDamage(int OldHealth, int NewHealth);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void OnTakeDamage_BP(int OldHealth, int NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Die();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Die1();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerDie1();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void ClientDie();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCanShoot(bool NewCanShoot);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSetCanShoot(bool NewCanShoot);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSetCanShoot(bool NewCanShoot);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual bool GetCanShoot();

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Functions")
	int CurrentWeaponIndex;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	TArray<AMultiplayerGun*> GetAllWeapons();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAllSharedCalibers(TMap<FName, int32> NewAllSharedCalibers);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TMap<FName, int32> GetAllSharedCalibers();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TArray<FName> GetAllSharedCaliberNames();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetSharedCaliberName(int32 Index, FName NewName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual FName GetSharedCaliberName(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual TArray<int32> GetAllSharedCaliberAmounts();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetSharedCaliberAmount(int32 Index, int32 NewAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int32 GetSharedCaliberAmount(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual AMultiplayerGun* GetWeapon(bool GetCurrentWeapon = true, int WeaponIndex = -1);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetWeaponIndex(AMultiplayerGun* Weapon);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetAmountOfWeapons();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual int GetMaxWeaponAmount();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetWeaponVisibility(bool ApplyToAllWeapons, int WeaponVisibilityToChange, bool Visible, bool SetAllOtherWeaponsToOppositeVisibility = true, bool ApplyToArms = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void GiveLoadout(TArray<TSubclassOf<AMultiplayerGun>> Loadout, int MaxWeaponAmount = 2);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerGiveLoadout(const TArray<TSubclassOf<AMultiplayerGun>>& Loadout, int MaxWeaponAmount = 2);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void GiveWeapon(TSubclassOf<AMultiplayerGun> WeaponToSpawn, AMultiplayerGun* WeaponToPickup = nullptr, bool SwitchToNewWeapon = false);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastGiveWeapon(AMultiplayerGun* WeaponToPickup = nullptr, bool SwitchToNewWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void AddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerAddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastAddWeapon(AMultiplayerGun* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void RemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerRemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastRemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon = false, AMultiplayerGun* Weapon = nullptr, int WeaponIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "This will also remove the weapon at the index you specify", ClampMin = 0))
	virtual void RemoveWeaponPastIndex(int WeaponIndex, bool DestroyWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SwitchWeaponsInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SwitchWeapons1();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerSwitchWeapons1();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastSwitchWeapons1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void NextWeapon();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerNextWeapon();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastNextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void LastWeapon();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerLastWeapon();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastLastWeapon();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SwitchToWeapon1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SwitchToWeapon2();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void PressFireInput();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ReleaseFireInput();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Fire();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void StopFiring(bool EvenCancelBurst = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void AimInput();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ReleaseAimInput();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Aim();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerAim();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastAim();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void StopAiming();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerStopAiming();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastStopAiming();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void SetAimingFOV_BP(bool Aiming, float AimingFOV, float TimeToAim);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetCanAim(bool NewCanAim);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetCanAim();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetIsAiming(bool NewIsAiming);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetIsAiming();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetIsADSing(bool NewIsADSing);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetIsADSing();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetIsZoomedIn(bool NewIsZoomedIn);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetIsZoomedIn();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetUseADS(int NewUseADS);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual int GetUseADS();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Reload();

	UFUNCTION(Server, Reliable, Category = "Functions")
	virtual void ServerReload();

	UFUNCTION(NetMulticast, Reliable, Category = "Functions")
	virtual void MulticastReload();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void Reload1();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	void ServerReload1();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	void MulticastReload1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void Reload2();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	void ServerReload2();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	void MulticastReload2();

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "Having PutArmsBackUp = false will not reset player model and gun reload animations"))
	virtual void CancelReload(bool PutArmsBackUp = true);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	void ServerCancelReload(bool PutArmsBackUp = true);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Functions")
	void MulticastCancelReload(bool PutArmsBackUp = true);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void Reload_BP();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void ShowHitMarker(AActor* HitActor, UPhysicalMaterial* HitPhysicalMaterial);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void ShowHitMarker_BP(AActor* HitActor, UPhysicalMaterial* HitPhysicalMaterial);

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "Set delay to 0 to not use it"))
	virtual void SetArmsAnimationMode(float Delay = 0.0f);

	UFUNCTION(Category = "Functions")
	virtual void SetArmsAnimationMode1();

	UFUNCTION(BlueprintCallable, Category = "Functions", meta = (Tooltip = "Set delay to 0 to not use it"))
	virtual void SetPlayerModelAnimationMode(float Delay = 0.0f);

	UFUNCTION(Category = "Functions")
	virtual void SetPlayerModelAnimationMode1();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = 0.0f, Tooltip = "Set this variable in the player controller, not here"))
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the X axis when not aiming, set this variable in the player controller, not here"))
	float MouseDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false, set this variable in the player controller, not here"))
	float MouseAimingSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the Y axis when not aiming, set this variable in the player controller, not here"))
	float MouseDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Mouse sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false, set this variable in the player controller, not here"))
	float MouseAimingSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Set this variable in the player controller, not here"))
	float MouseAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the X axis when not aiming, set this variable in the player controller, not here"))
	float GamepadDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false, set this variable in the player controller, not here"))
	float GamepadAimingSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the Y axis when not aiming, set this variable in the player controller, not here"))
	float GamepadDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Gamepad sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false, set this variable in the player controller, not here"))
	float GamepadAimingSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Set this variable in the player controller, not here"))
	float GamepadAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Set this variable in the player controller, not here"))
	bool UseAimSensitivityMultipler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Set this variable in the player controller, not here"))
	bool ToggleAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Tooltip = "Set this variable in the player controller, not here"))
	bool HoldButtonToJump;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	int UseActorClassesForHitMarkers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effects")
	bool CanGetHitMarkersOnSelf;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	TMap<TSubclassOf<AActor>, USoundBase*> HitMarkerActorSounds;

	UPROPERTY(BlueprintReadWrite, Category = "Hit Effects")
	TMap<UPhysicalMaterial*, USoundBase*> HitMarkerSurfaceSounds;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	int PlayerIndex;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	APlayerController* OwningController;

protected:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	AMultiplayerPlayerController* OwningControllerCast;

public:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	FVector ReplicatedCameraLocation;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	FRotator ReplicatedCameraRotation;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Player Info")
	FRotator ReplicatedControlRotation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMultiplayerHealthComponent* HealthComponent;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Perspective", meta = (Tooltip = "Set this to true if you want third person to be default, set this in the player controller"))
	bool UsingThirdPerson;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float FirstPersonSpringArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float ThirdPersonSpringArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	FVector FirstPersonSpringArmLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	FVector ThirdPersonSpringArmLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool AttachSpringArmToPlayerModelFirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	FName SocketToAttachSpringArmToFirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	USkeletalMesh* FirstPersonPlayerModelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	USkeletalMesh* FirstPersonPlayerModelWithoutWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool FirstPersonCameraLag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool FirstPersonCameraRotationLag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float FirstPersonCameraLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float FirstPersonCameraRotationLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool ThirdPersonCameraLag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool ThirdPersonCameraRotationLag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float ThirdPersonCameraLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float ThirdPersonCameraRotationLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	float PerspectiveTransitionTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool HidePlayerModelMeshInFirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective", meta = (Tooltip = "This will override HideFirstPersonArmsWithoutWeapon"))
	bool HideFirstPersonArmsAndGunInFirstPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective", meta = (Tooltip = "This will decide whether to hide the first person arms mesh when you do not have a weapon"))
	bool HideFirstPersonArmsWithoutWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perspective")
	bool HideThirdPersonGunInFirstPerson;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interact")
	bool CanInteract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (ClampMin = 0.0f))
	float InteractDistance;

	UPROPERTY(BlueprintReadWrite, Category = "Interact")
	bool OverlappingInteractable;

	UPROPERTY(BlueprintReadWrite, Category = "Interact")
	AInteractableItem* InteractableBeingOverlapped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons")
	bool CanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool ShootingCancelsReload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This will determine whether weapons are dropped or destroyed on death, 0 = destroy weapons, 1 = drop only the weapon being held, 2 = drop all weapons", ClampMin = 0, ClampMax = 2))
	int DropWeaponsOnDeath;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	bool HoldingFireInput;

	UPROPERTY(BlueprintReadWrite, Category = "Variables", meta = (Tooltip = "This is only used for the default animation blueprint"))
	bool HoldingJumpInput;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	bool HoldingAimInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons")
	bool CanAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool AimingCancelsReload;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	bool IsAiming;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	bool IsADSing;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	bool IsZoomedIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (Tooltip = "0 = determined by weapon, 1 = ADS with all weapons, 2 = zoom in with all weapons", ClampMin = 0, ClampMax = 2))
	int UseADS;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	bool IsSwitchingWeapons;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	float CurrentFOV;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	float CurrentMouseSensitivityX;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	float CurrentMouseSensitivityY;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	float CurrentGamepadSensitivityX;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	float CurrentGamepadSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	FName HeadSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Used on death to properly locate the camera when it attaches to the head"))
	FVector CameraHeadLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (Tooltip = "Used on death to properly rotate the camera when it attaches to the head"))
	FRotator CameraHeadRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations", meta = (ClampMin = 0.0f))
	float DelayToDestroyBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool CanReload;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	bool IsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ReloadCharacterSound;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	TArray<AMultiplayerGun*> AllWeapons;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (Tooltip = "Only applies to guns that use this rather than their own reserve ammo, set defaults in the player controller", ClampMin = 0))
	TArray<FName> AllSharedCaliberNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (Tooltip = "Only applies to guns that use this rather than their own reserve ammo, set defaults in the player controller", ClampMin = 0))
	TArray<int32> AllSharedCaliberAmounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UCameraShakeBase> TakeDamageCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Clear this to disable"))
	UForceFeedbackEffect* TakeDamageControllerVibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Used to cancel vibration with continuous fire"))
	FName TakeDamageControllerVibrationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UCameraShakeBase> DieDamageCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Clear this to disable"))
	UForceFeedbackEffect* DieControllerVibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (Tooltip = "Used to cancel vibration with continuous fire"))
	FName DieControllerVibrationTag;

	FTimerHandle GetPlayerControllerTimerHandle;
	FTimerHandle DestroySelfTimerHandle;
	FTimerDelegate SwitchWeaponsTimerDelegate;
	FTimerHandle SwitchWeaponsTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle ArmsAnimationModeTimerHandle;
	FTimerHandle PlayerModelAnimationModeTimerHandle;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};