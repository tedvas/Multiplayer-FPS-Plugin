// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "MultiplayerPlayerController.h"
#include "TimerManager.h"

// Sets default values
AMultiplayerCharacter::AMultiplayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	/* ************* Components ************* */
	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera Component"));
	FPCameraComponent->SetupAttachment(RootComponent, NAME_None);
	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms Mesh"));
	ArmsMesh->SetupAttachment(FPCameraComponent, NAME_None);


	/* ************* Actor Components ************* */
	HealthComponent = CreateDefaultSubobject<UMultiplayerHealthComponent>(TEXT("Health Component"));


	/* ************* Sensitivity ************* */
	MouseDefaultSensitivityX = 0.5f;
	MouseAimingSensitivityX = 0.275f;
	CurrentMouseSensitivityX = 0.5f;
	MouseDefaultSensitivityY = 0.5f;
	MouseAimingSensitivityY = 0.275f;
	CurrentMouseSensitivityY = 0.5f;
	MouseAimingSensitivityMultiplier = 0.55f;
	GamepadDefaultSensitivityX = 0.5f;
	GamepadAimingSensitivityX = 0.275f;
	CurrentGamepadSensitivityX = 0.5f;
	GamepadDefaultSensitivityY = 0.5f;
	GamepadAimingSensitivityY = 0.275f;
	CurrentGamepadSensitivityY = 0.5f;
	GamepadAimingSensitivityMultiplier = 0.55f;


	/* ************* Settings ************* */
	FieldOfView = 90.0f;
	CurrentFOV = 90.0f;
	UseAimSensitivityMultipler = true;
	ToggleAim = false;
	HoldButtonToJump = false;


	/* ************* Interact ************* */
	CanInteract = true;
	InteractDistance = 175.0f;
	OverlappingWeapon = false;


	/* ************* Weapons ************* */
	CanShoot = true;
	ShootingCancelsReload = true;
	CurrentWeaponIndex = 0;
	DropWeaponsOnDeath = true;
	TimeToSwitchWeapons = 0.9f;
	IsSwitchingWeapons = false;
	CanReload = true;
	IsReloading = false;
	HoldingFireInput = false;
	HoldingAimInput = false;
	CanAim = true;
	AimingCancelsReload = true;
	IsAiming = false;
	IsADSing = false;
	IsZoomedIn = false;
	UseADS = 0;


	/* ************* Input ************* */
	IMC_Priority = 0;


	/* ************* Animations ************* */
	HeadSocketName = "head";
	CameraHeadLocation = FVector(2.0f, 7.5f, 0.0f);
	CameraHeadRotation = FRotator(0.0f, 90.0f, -90.0f);
	RespawnDelay = 2.5f;
	DelayToDestroyBody = 10.0f;
	UseTwoWeaponSwitchAnimations = true;


	bReplicates = true;
}

UMultiplayerHealthComponent* AMultiplayerCharacter::GetHealthComponent()
{
	return HealthComponent;
}

void AMultiplayerCharacter::SetupInput_Implementation()
{
	if (OwningController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Default, IMC_Priority);
		}
	}
}

void AMultiplayerCharacter::RemoveInput_Implementation()
{
	if (OwningController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(IMC_Default);
		}
	}
}

void AMultiplayerCharacter::GetOwningController_Implementation()
{
	// Because this is running on the client it is okay for the player index to be 0
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex))
	{
		OwningController = PlayerController;

		if (AMultiplayerPlayerController* MultiplayerController = Cast<AMultiplayerPlayerController>(OwningController))
		{
			PlayerIndex = MultiplayerController->GetPlayerIndex();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "MultiplayerPlayerController Cast Failed, If You Are Not Using The MultiplayerPlayerController You Can Ignore This; MultiplayerCharacter.cpp:GetOwningController");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Failed To Get Player Controller; MultiplayerCharacter.cpp:GetOwningController");
	}
}

void AMultiplayerCharacter::RecalculateBaseEyeHeight()
{
	BaseEyeHeight = FPCameraComponent->GetRelativeLocation().Z;
}

void AMultiplayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AMultiplayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVector.X * CurrentMouseSensitivityX);
		AddControllerPitchInput(LookVector.Y * CurrentMouseSensitivityY);
	}
}

void AMultiplayerCharacter::GamepadLook(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookVector.X * CurrentGamepadSensitivityX);
		AddControllerPitchInput(LookVector.Y * CurrentGamepadSensitivityY);
	}
}

void AMultiplayerCharacter::PressJump()
{
	if (HoldButtonToJump == false)
	{
		Jump();
	}
}

void AMultiplayerCharacter::HoldJump()
{
	if (HoldButtonToJump == true)
	{
		Jump();
	}
}

void AMultiplayerCharacter::SetSensitivity()
{
	if (IsAiming == false)
	{
		CurrentMouseSensitivityX = MouseDefaultSensitivityX;
		CurrentMouseSensitivityY = MouseDefaultSensitivityY;

		CurrentGamepadSensitivityX = GamepadDefaultSensitivityX;
		CurrentGamepadSensitivityY = GamepadDefaultSensitivityY;
	}
	else
	{
		if (UseAimSensitivityMultipler == true)
		{
			CurrentMouseSensitivityX = MouseDefaultSensitivityX * MouseAimingSensitivityMultiplier;
			CurrentMouseSensitivityY = MouseDefaultSensitivityY * MouseAimingSensitivityMultiplier;

			CurrentGamepadSensitivityX = GamepadDefaultSensitivityX * GamepadAimingSensitivityMultiplier;
			CurrentGamepadSensitivityY = GamepadDefaultSensitivityY * GamepadAimingSensitivityMultiplier;
		}
		else
		{
			CurrentMouseSensitivityX = MouseAimingSensitivityX;
			CurrentMouseSensitivityY = MouseAimingSensitivityY;

			CurrentGamepadSensitivityX = GamepadAimingSensitivityX;
			CurrentGamepadSensitivityY = GamepadAimingSensitivityY;
		}
	}
}

void AMultiplayerCharacter::Interact()
{
	if (!HasAuthority())
	{
		ServerInteract();
	}

	if (CanInteract == true)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector TraceDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (TraceDirection * InteractDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		for (auto& Weapon : AllWeapons)
		{
			if (Weapon)
			{
				QueryParams.AddIgnoredActor(Weapon);
			}
		}

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			if (AInteractableItem* InteractableItemCast = Cast<AInteractableItem>(HitActor))
			{
				PickupItem(InteractableItemCast);
			}
		}
		else if (OverlappingWeapon == true && WeaponBeingOverlapped)
		{
			PickupItem(WeaponBeingOverlapped);
		}
	}
}

void AMultiplayerCharacter::ServerInteract_Implementation()
{
	Interact();
}

void AMultiplayerCharacter::PickupItem(AInteractableItem* ItemToPickup)
{
	if (!HasAuthority())
	{
		ServerPickupItem(ItemToPickup);
	}

	if (ItemToPickup)
	{
		if (AMultiplayerGun* GunCast = Cast<AMultiplayerGun>(ItemToPickup))
		{
			StopFiring(true);

			if (GetAmountOfWeapons() >= GetMaxWeaponAmount())
			{
				RemoveWeapon(false, false, GetWeapon(true));
			}

			GiveWeapon(nullptr, GunCast, true);
			return;
		}
	}
}

void AMultiplayerCharacter::ServerPickupItem_Implementation(AInteractableItem* ItemToPickup)
{
	PickupItem(ItemToPickup);
}

void AMultiplayerCharacter::SetCanInteract(bool NewCanInteract)
{
	if (HasAuthority())
	{
		MulticastSetCanInteract(NewCanInteract);
	}
	else
	{
		ServerSetCanInteract(NewCanInteract);
	}
}

void AMultiplayerCharacter::ServerSetCanInteract_Implementation(bool NewCanInteract)
{
	MulticastSetCanInteract(NewCanInteract);
}

void AMultiplayerCharacter::MulticastSetCanInteract_Implementation(bool NewCanInteract)
{
	CanInteract = NewCanInteract;
}

bool AMultiplayerCharacter::GetCanInteract()
{
	return CanInteract;
}

void AMultiplayerCharacter::SetInteractDistance(float NewInteractDistance)
{
	InteractDistance = NewInteractDistance;
}

float AMultiplayerCharacter::GetInteractDistance()
{
	return InteractDistance;
}

void AMultiplayerCharacter::SetOverlappingWeapon(bool NewOverlappingWeapon)
{
	OverlappingWeapon = NewOverlappingWeapon;
}

bool AMultiplayerCharacter::GetOverlappingWeapon()
{
	return OverlappingWeapon;
}

void AMultiplayerCharacter::SetWeaponBeingOverlapped(AMultiplayerGun* NewWeaponBeingOverlapped)
{
	WeaponBeingOverlapped = NewWeaponBeingOverlapped;
}

AMultiplayerGun* AMultiplayerCharacter::GetWeaponBeingOverlapped()
{
	return WeaponBeingOverlapped;
}

void AMultiplayerCharacter::Die()
{
	Die1();
}

void AMultiplayerCharacter::Die1()
{
	if (!HasAuthority())
	{
		ServerDie1();
	}

	if (GetMesh())
	{
		ArmsMesh->SetHiddenInGame(true);
		ArmsMesh->SetVisibility(false);

		FPCameraComponent->bUsePawnControlRotation = false;
		FPCameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, HeadSocketName);
		FPCameraComponent->SetRelativeLocationAndRotation(CameraHeadLocation, CameraHeadRotation);

		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetOwnerNoSee(false);
	}

	if (GetController())
	{
		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(GetController()))
		{
			if (RespawnDelay > DelayToDestroyBody)
			{
				DelayToDestroyBody = RespawnDelay;
			}

			ControllerCast->Respawn(RespawnDelay);
		}
	}

	if (DropWeaponsOnDeath == true)
	{
		RemoveWeapon(true);
	}
	else
	{
		RemoveWeapon(true, true);
	}

	GetHealthComponent()->DestroyComponent();

	GetWorldTimerManager().SetTimer(DestroySelfTimerHandle, this, &AMultiplayerCharacter::DestroySelf, DelayToDestroyBody, false, DelayToDestroyBody);
}

void AMultiplayerCharacter::ServerDie1_Implementation()
{
	Die();
}

void AMultiplayerCharacter::ClientDie_Implementation()
{
	RemoveInput();

	if (OwningController)
	{
		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(OwningController))
		{
			if (ControllerCast->DeathScreenClass)
			{
				ControllerCast->CreateUIWidget(ControllerCast->DeathScreenClass, 0, false, ControllerCast->HUD);
			}
		}
	}
}

void AMultiplayerCharacter::DestroySelf_Implementation()
{
	Destroy();
}

void AMultiplayerCharacter::SetCanShoot(bool NewCanShoot)
{
	if (HasAuthority())
	{
		MulticastSetCanShoot(NewCanShoot);
	}
	else
	{
		ServerSetCanShoot(NewCanShoot);
	}
}

void AMultiplayerCharacter::ServerSetCanShoot_Implementation(bool NewCanShoot)
{
	MulticastSetCanShoot(NewCanShoot);
}

void AMultiplayerCharacter::MulticastSetCanShoot_Implementation(bool NewCanShoot)
{
	CanShoot = NewCanShoot;

	if (CanShoot == false)
	{
		StopFiring(true);
	}
}

bool AMultiplayerCharacter::GetCanShoot()
{
	return CanShoot;
}

TArray<AMultiplayerGun*> AMultiplayerCharacter::GetAllWeapons()
{
	return AllWeapons;
}

AMultiplayerGun* AMultiplayerCharacter::GetWeapon(bool GetCurrentWeapon, int WeaponIndex)
{
	if (GetAmountOfWeapons() >= 1)
	{
		if (GetCurrentWeapon == true || WeaponIndex <= -1)
		{
			if (AllWeapons.IsValidIndex(CurrentWeaponIndex) == true)
			{
				return AllWeapons[CurrentWeaponIndex];
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			if ((AllWeapons.IsValidIndex(WeaponIndex) == true))
			{
				return AllWeapons[WeaponIndex];
			}
			else
			{
				return nullptr;
			}
		}
	}
	else
	{
		return nullptr;
	}
}

int AMultiplayerCharacter::GetWeaponIndex(AMultiplayerGun* Weapon)
{
	if (GetAmountOfWeapons() >= 1)
	{
		if (AllWeapons.Contains(Weapon))
		{
			return AllWeapons.Find(Weapon);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

int AMultiplayerCharacter::GetAmountOfWeapons()
{
	return AllWeapons.Num();
}

int AMultiplayerCharacter::GetMaxWeaponAmount()
{
	if (OwningController)
	{
		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(OwningController))
		{
			return ControllerCast->GetMaxWeaponAmount();
		}
		else
		{
			if (HasAuthority())
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Host: Controller Cast Failed MultiplayerCharacter.cpp:GetMaxWeaponAmount");
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Client: Controller Cast Failed MultiplayerCharacter.cpp:GetMaxWeaponAmount");
			}

			return -1;
		}
	}
	else
	{
		if (HasAuthority())
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Host: OwningController Invalid Failed MultiplayerCharacter.cpp:GetMaxWeaponAmount");
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Client: OwningController Invalid Failed MultiplayerCharacter.cpp:GetMaxWeaponAmount");
		}

		return -1;
	}
}

void AMultiplayerCharacter::SetWeaponVisibility(int WeaponVisibilityToChange, bool Visible, bool SetAllOtherWeaponsToOppositeVisibility)
{
	if (GetAmountOfWeapons() >= 1 && AllWeapons.Contains(GetWeapon(false, WeaponVisibilityToChange)) == true)
	{
		if (SetAllOtherWeaponsToOppositeVisibility == true)
		{
			for (int32 Index = 0; Index != AllWeapons.Num(); ++Index)
			{
				if (Index != WeaponVisibilityToChange)
				{
					GetWeapon(false, Index)->SetActorHiddenInGame(Visible);
				}
			}
		}

		GetWeapon(false, WeaponVisibilityToChange)->SetActorHiddenInGame(!Visible);
	}
}

void AMultiplayerCharacter::GiveLoadout(TArray<TSubclassOf<AMultiplayerGun>> Loadout, int MaxWeaponAmount)
{
	for (auto& Weapon : Loadout)
	{
		if (GetAmountOfWeapons() < MaxWeaponAmount && Weapon)
		{
			GiveWeapon(Weapon, nullptr);
		}
	}

	SwitchWeapons(0);
}

void AMultiplayerCharacter::GiveWeapon_Implementation(TSubclassOf<AMultiplayerGun> WeaponToSpawn, AMultiplayerGun* WeaponToPickup, bool SwitchToNewWeapon)
{
	if (GetAmountOfWeapons() < GetMaxWeaponAmount())
	{
		if (WeaponToSpawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			WeaponToPickup = GetWorld()->SpawnActor<AMultiplayerGun>(WeaponToSpawn, GetActorLocation(), GetActorRotation(), SpawnParams);
		}

		if (WeaponToPickup)
		{
			WeaponToPickup->SetOwner(this);
			WeaponToPickup->SetOwningPlayer(this);
			WeaponToPickup->SetWasPickedup(true, ArmsMesh);

			AddWeapon(WeaponToPickup);

			if (SwitchToNewWeapon == true || GetAmountOfWeapons() <= 0)
			{
				SwitchWeapons(GetWeaponIndex(WeaponToPickup));
			}
		}
	}
}

void AMultiplayerCharacter::AddWeapon(AMultiplayerGun* Weapon)
{
	if (HasAuthority())
	{
		MulticastAddWeapon(Weapon);
	}
	else
	{
		ServerAddWeapon(Weapon);
	}
}

void AMultiplayerCharacter::ServerAddWeapon_Implementation(AMultiplayerGun* Weapon)
{
	MulticastAddWeapon(Weapon);
}

void AMultiplayerCharacter::MulticastAddWeapon_Implementation(AMultiplayerGun* Weapon)
{
	if (Weapon)
	{
		AllWeapons.Add(Weapon);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Weapon To Add Not Valid FirstPersonCharacterCPP.cpp:AddWeapon");
	}
}

void AMultiplayerCharacter::RemoveWeapon(bool RemoveAllWeapons, bool DestroyWeapon, AMultiplayerGun* Weapon, int WeaponIndex)
{
	if (HasAuthority())
	{
		MulticastRemoveWeapon(RemoveAllWeapons, DestroyWeapon, Weapon, WeaponIndex);
	}
	else
	{
		ServerRemoveWeapon(RemoveAllWeapons, DestroyWeapon, Weapon, WeaponIndex);
	}
}

void AMultiplayerCharacter::ServerRemoveWeapon_Implementation(bool RemoveAllWeapons, bool DestroyWeapon, AMultiplayerGun* Weapon, int WeaponIndex)
{
	MulticastRemoveWeapon(RemoveAllWeapons, DestroyWeapon, Weapon, WeaponIndex);
}

void AMultiplayerCharacter::MulticastRemoveWeapon_Implementation(bool RemoveAllWeapons, bool DestroyWeapon, AMultiplayerGun* Weapon, int WeaponIndex)
{
	if (RemoveAllWeapons == false)
	{
		if (Weapon)
		{
			bool SwitchToNewWeapon;

			if (Weapon == GetWeapon(true) && GetAmountOfWeapons() > 1)
			{
				SwitchToNewWeapon = true;

				if (CurrentWeaponIndex >= GetAmountOfWeapons() - 1)
				{
					CurrentWeaponIndex = 0;
				}
			}
			else
			{
				if (CurrentWeaponIndex > 0)
				{
					CurrentWeaponIndex--;
				}

				SwitchToNewWeapon = false;
			}

			Weapon->SetOwner(nullptr);
			Weapon->SetOwningPlayer(nullptr);
			AllWeapons.Remove(Weapon);

			if (DestroyWeapon == false)
			{
				Weapon->SetWasPickedup(false);
			}
			else
			{
				Weapon->Destroy();
			}

			if (SwitchToNewWeapon == true)
			{
				SwitchWeapons(CurrentWeaponIndex);
			}
		}
		else if (WeaponIndex >= 0 && AllWeapons.IsValidIndex(WeaponIndex) == true)
		{
			AMultiplayerGun* WeaponToRemove = AllWeapons[WeaponIndex];

			bool SwitchToNewWeapon;

			if (WeaponToRemove == GetWeapon(true) && GetAmountOfWeapons() > 1)
			{
				SwitchToNewWeapon = true;

				if (CurrentWeaponIndex >= GetAmountOfWeapons() - 1)
				{
					CurrentWeaponIndex = 0;
				}
			}
			else
			{
				if (CurrentWeaponIndex > 0)
				{
					CurrentWeaponIndex--;
				}

				SwitchToNewWeapon = false;
			}

			WeaponToRemove->SetOwner(nullptr);
			WeaponToRemove->SetOwningPlayer(nullptr);
			AllWeapons.Remove(WeaponToRemove);

			if (DestroyWeapon == false)
			{
				WeaponToRemove->SetWasPickedup(false);
			}
			else
			{
				WeaponToRemove->Destroy();
			}

			if (SwitchToNewWeapon == true)
			{
				SwitchWeapons(CurrentWeaponIndex);
			}
		}
	}
	else
	{
		for (auto& WeaponToRemove : AllWeapons)
		{
			if (WeaponToRemove && AllWeapons.Contains(WeaponToRemove) == true)
			{
				if (DestroyWeapon == true)
				{
					WeaponToRemove->SetOwner(nullptr);
					WeaponToRemove->SetOwningPlayer(nullptr);
					WeaponToRemove->Destroy();
				}
				else
				{
					WeaponToRemove->SetWasPickedup(false);
					WeaponToRemove->SetOwner(nullptr);
					WeaponToRemove->SetOwningPlayer(nullptr);
				}
			}
		}

		AllWeapons.Empty();
	}
}

void AMultiplayerCharacter::SwitchWeaponsInput(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();

	if (InputValue == 1)
	{
		NextWeapon();
	}
	else
	{
		LastWeapon();
	}
}

void AMultiplayerCharacter::NextWeapon()
{
	if (HasAuthority())
	{
		MulticastNextWeapon();
	}
	else
	{
		ServerNextWeapon();
	}
}

void AMultiplayerCharacter::ServerNextWeapon_Implementation()
{
	MulticastNextWeapon();
}

void AMultiplayerCharacter::MulticastNextWeapon_Implementation()
{
	GetWorldTimerManager().ClearTimer(SwitchWeaponsTimerHandle);

	if (GetAmountOfWeapons() >= 2)
	{
		SetCanShoot(false);
		SetCanAim(false);
		StopFiring(true);
		StopAiming();

		if (WeaponSwitchAnimation)
		{
			ArmsMesh->PlayAnimation(WeaponSwitchAnimation, false);
		}

		if (CurrentWeaponIndex >= GetAmountOfWeapons() - 1)
		{
			CurrentWeaponIndex = 0;
		}
		else
		{
			CurrentWeaponIndex++;
		}

		if (IsSwitchingWeapons == false && IsReloading == false)
		{
			IsSwitchingWeapons = true;

			SwitchWeaponsTimerDelegate.BindUFunction(this, "SwitchWeapons", CurrentWeaponIndex, nullptr);
			GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, SwitchWeaponsTimerDelegate, TimeToSwitchWeapons / 2, false);
		}
		else
		{
			CancelReload(false);
			SwitchWeapons(CurrentWeaponIndex);
		}
	}
}

void AMultiplayerCharacter::LastWeapon()
{
	if (HasAuthority())
	{
		MulticastLastWeapon();
	}
	else
	{
		ServerLastWeapon();
	}
}

void AMultiplayerCharacter::ServerLastWeapon_Implementation()
{
	MulticastLastWeapon();
}

void AMultiplayerCharacter::MulticastLastWeapon_Implementation()
{
	GetWorldTimerManager().ClearTimer(SwitchWeaponsTimerHandle);

	if (GetAmountOfWeapons() >= 2)
	{
		SetCanShoot(false);
		SetCanAim(false);
		StopFiring(true);
		StopAiming();

		if (WeaponSwitchAnimation)
		{
			ArmsMesh->PlayAnimation(WeaponSwitchAnimation, false);
		}

		if (CurrentWeaponIndex <= 0)
		{
			CurrentWeaponIndex = GetAmountOfWeapons() - 1;
		}
		else
		{
			CurrentWeaponIndex--;
		}

		if (IsSwitchingWeapons == false && IsReloading == false)
		{
			IsSwitchingWeapons = true;

			SwitchWeaponsTimerDelegate.BindUFunction(this, "SwitchWeapons", CurrentWeaponIndex, nullptr);
			GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, SwitchWeaponsTimerDelegate, TimeToSwitchWeapons / 2, false);
		}
		else
		{
			CancelReload(false);
			SwitchWeapons(CurrentWeaponIndex);
		}
	}
}

void AMultiplayerCharacter::SwitchWeapons(int Index, AMultiplayerGun* WeaponToSwitchTo)
{
	if (HasAuthority())
	{
		MulticastSwitchWeapons(Index, WeaponToSwitchTo);
	}
	else
	{
		ServerSwitchWeapons(Index, WeaponToSwitchTo);
	}
}

void AMultiplayerCharacter::ServerSwitchWeapons_Implementation(int Index, AMultiplayerGun* WeaponToSwitchTo)
{
	MulticastSwitchWeapons(Index, WeaponToSwitchTo);
}

void AMultiplayerCharacter::MulticastSwitchWeapons_Implementation(int Index, AMultiplayerGun* WeaponToSwitchTo)
{
	if (GetAmountOfWeapons() >= 1)
	{
		SetCanShoot(false);
		SetCanAim(false);
		StopFiring(true);
		StopAiming();
		CancelReload(false);

		if (UseTwoWeaponSwitchAnimations == true && WeaponSwitchAnimation1)
		{
			ArmsMesh->PlayAnimation(WeaponSwitchAnimation1, false);
		}

		if (!WeaponToSwitchTo)
		{
			CurrentWeaponIndex = Index;
		}
		else
		{
			CurrentWeaponIndex = GetWeaponIndex(WeaponToSwitchTo);
		}

		if (AMultiplayerGun* Gun = GetWeapon(true))
		{
			ArmsMesh->SetRelativeLocationAndRotation(Gun->GetPlayerArmsRelativeLocation(), Gun->GetPlayerArmsRelativeRotation());
		}

		SetWeaponVisibility(CurrentWeaponIndex, true);

		GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerCharacter::SwitchWeapons1, TimeToSwitchWeapons / 2, false, TimeToSwitchWeapons / 2);
	}
}

void AMultiplayerCharacter::SwitchWeapons1()
{
	if (HasAuthority())
	{
		MulticastSwitchWeapons1();
	}
	else
	{
		ServerSwitchWeapons1();
	}
}

void AMultiplayerCharacter::ServerSwitchWeapons1_Implementation()
{
	MulticastSwitchWeapons1();
}

void AMultiplayerCharacter::MulticastSwitchWeapons1_Implementation()
{
	IsSwitchingWeapons = false;

	if (GetAmountOfWeapons() >= 1)
	{
		ArmsMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);

		SetCanShoot(true);
		SetCanAim(true);

		if (GetWeapon(true))
		{
			if (GetWeapon(true)->GetAmmoInMagazine() <= 0)
			{
				if (ReloadAnimation)
				{
					ArmsMesh->PlayAnimation(ReloadAnimation, false);
				}

				Reload();
			}
			else if (GetWeapon(true)->GetFireMode() != 0 && HoldingFireInput == true)
			{
				Fire();
			}
		}

		if (HoldingAimInput == true)
		{
			Aim();
		}
	}
}

void AMultiplayerCharacter::PressFireInput()
{
	HoldingFireInput = true;

	Fire();
}

void AMultiplayerCharacter::ReleaseFireInput()
{
	HoldingFireInput = false;

	StopFiring();
}

void AMultiplayerCharacter::Fire()
{
	if (GetAmountOfWeapons() >= 1)
	{
		if (GetCanShoot() == true && GetWeapon(true))
		{
			if (GetWeapon(true)->GetAmmoInMagazine() >= 1)
			{
				if (ShootingCancelsReload == true || IsReloading == false)
				{
					CancelReload();

					GetWeapon(true)->FireInput();
				}
			}
		}
	}
}

void AMultiplayerCharacter::StopFiring(bool EvenCancelBurst)
{
	if (GetWeapon(true))
	{
		GetWeapon(true)->StopFiring(EvenCancelBurst);
	}
}

void AMultiplayerCharacter::AimInput()
{
	if (ToggleAim == false)
	{
		HoldingAimInput = true;

		Aim();
	}
	else
	{
		if (IsAiming == true)
		{
			HoldingAimInput = false;

			StopAiming();
		}
		else
		{
			Aim();
		}
	}
}

void AMultiplayerCharacter::ReleaseAimInput()
{
	if (ToggleAim == false)
	{
		HoldingAimInput = false;

		StopAiming();
	}
}

void AMultiplayerCharacter::Aim()
{
	if (CanAim == true && IsAiming == false && GetWeapon(true))
	{
		if (AimingCancelsReload == true || IsReloading == false)
		{
			CancelReload();

			IsAiming = true;

			SetSensitivity();

			AMultiplayerGun* CurrentWeapon = GetWeapon(true);

			FVector AimLocation;
			FRotator AimRotation;

			if (UseADS == 0 && CurrentWeapon->GetUseADS() == 0 || CurrentWeapon->GetUseADS() == 2 || UseADS == 1 && CurrentWeapon->GetUseADS() < 2)
			{
				IsADSing = true;

				AimLocation = CurrentWeapon->GetADSArmsLocation();
				AimRotation = CurrentWeapon->GetADSArmsRotation();

				SetAimingFOV_BP(true, FieldOfView - GetWeapon(true)->GetADSFOV(), GetWeapon(true)->GetTimeToADS());

				FLatentActionInfo LatentActionInfo;
				LatentActionInfo.CallbackTarget = this;

				UKismetSystemLibrary::MoveComponentTo(ArmsMesh, AimLocation, AimRotation, false, false, CurrentWeapon->GetTimeToADS(), false, EMoveComponentAction::Move, LatentActionInfo);
			}
			else
			{
				IsZoomedIn = true;

				AimLocation = CurrentWeapon->GetZoomArmsLocation();
				AimRotation = CurrentWeapon->GetZoomArmsRotation();

				SetAimingFOV_BP(true, FieldOfView - GetWeapon(true)->GetZoomFOV(), GetWeapon(true)->GetTimeToZoom());

				FLatentActionInfo LatentActionInfo;
				LatentActionInfo.CallbackTarget = this;

				UKismetSystemLibrary::MoveComponentTo(ArmsMesh, AimLocation, AimRotation, false, false, CurrentWeapon->GetTimeToZoom(), false, EMoveComponentAction::Move, LatentActionInfo);
			}
		}
	}
}

void AMultiplayerCharacter::StopAiming()
{
	if (GetWeapon(true))
	{
		if (IsAiming == true || IsADSing == true || IsZoomedIn == true)
		{
			IsAiming = false;
			IsADSing = false;
			IsZoomedIn = false;

			SetSensitivity();

			AMultiplayerGun* CurrentWeapon = GetWeapon(true);

			float AimTime;

			if (UseADS == 0 && CurrentWeapon->GetUseADS() == 0 || CurrentWeapon->GetUseADS() == 2 || UseADS == 1 && CurrentWeapon->GetUseADS() < 2)
			{
				AimTime = CurrentWeapon->GetTimeToADS();
				SetAimingFOV_BP(false, FieldOfView - CurrentWeapon->GetADSFOV(), AimTime);
			}
			else
			{
				AimTime = CurrentWeapon->GetTimeToZoom();
				SetAimingFOV_BP(false, FieldOfView - CurrentWeapon->GetZoomFOV(), AimTime);
			}

			FLatentActionInfo LatentActionInfo;
			LatentActionInfo.CallbackTarget = this;

			UKismetSystemLibrary::MoveComponentTo(ArmsMesh, CurrentWeapon->GetPlayerArmsRelativeLocation(), CurrentWeapon->GetPlayerArmsRelativeRotation(), false, false, AimTime, false, EMoveComponentAction::Move, LatentActionInfo);
		}
	}
}

void AMultiplayerCharacter::SetCanAim(bool NewCanAim)
{
	CanAim = NewCanAim;
}

bool AMultiplayerCharacter::GetCanAim()
{
	return CanAim;
}

void AMultiplayerCharacter::SetIsAiming(bool NewIsAiming)
{
	IsAiming = NewIsAiming;
}

bool AMultiplayerCharacter::GetIsAiming()
{
	return IsAiming;
}

void AMultiplayerCharacter::SetIsADSing(bool NewIsADSing)
{
	IsADSing = NewIsADSing;
}

bool AMultiplayerCharacter::GetIsADSing()
{
	return IsADSing;
}

void AMultiplayerCharacter::SetIsZoomedIn(bool NewIsZoomedIn)
{
	IsZoomedIn = NewIsZoomedIn;
}

bool AMultiplayerCharacter::GetIsZoomedIn()
{
	return IsZoomedIn;
}

void AMultiplayerCharacter::SetUseADS(int NewUseADS)
{
	UseADS = NewUseADS;
}

int AMultiplayerCharacter::GetUseADS()
{
	return UseADS;
}

void AMultiplayerCharacter::Reload()
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

void AMultiplayerCharacter::ServerReload_Implementation()
{
	MulticastReload();
}

void AMultiplayerCharacter::MulticastReload_Implementation()
{
	if (GetAmountOfWeapons() >= 1 && CanReload == true && IsReloading == false)
	{
		if (GetWeapon(true))
		{
			if (GetWeapon(true)->GetReserveAmmo() >= 1 && GetWeapon(true)->GetAmmoInMagazine() < GetWeapon(true)->GetMaxAmmoInMagazine())
			{
				StopFiring(true);
				StopAiming();

				if (GetWeapon(true)->GetAmmoInMagazine() <= 0)
				{
					CanAim = false;
				}

				IsReloading = true;

				if (ReloadAnimation)
				{
					ArmsMesh->PlayAnimation(ReloadAnimation, false);
				}

				GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerCharacter::Reload1, GetWeapon(true)->ReloadSpeed, false, GetWeapon(true)->ReloadSpeed);
			}
		}
	}
}

void AMultiplayerCharacter::Reload1()
{
	if (HasAuthority())
	{
		MulticastReload1();
	}
	else
	{
		ServerReload1();
	}
}

void AMultiplayerCharacter::ServerReload1_Implementation()
{
	MulticastReload1();
}

void AMultiplayerCharacter::MulticastReload1_Implementation()
{
	if (GetWeapon(true))
	{
		GetWeapon(true)->Reload();
	}

	IsReloading = false;
	CanAim = true;

	if (ReloadAnimation1)
	{
		ArmsMesh->PlayAnimation(ReloadAnimation1, false);
	}

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerCharacter::Reload2, GetWeapon(true)->ReloadSpeed1, false, GetWeapon(true)->ReloadSpeed1);
}

void AMultiplayerCharacter::Reload2()
{
	if (HasAuthority())
	{
		MulticastReload2();
	}
	else
	{
		ServerReload2();
	}
}

void AMultiplayerCharacter::ServerReload2_Implementation()
{
	MulticastReload2();
}

void AMultiplayerCharacter::MulticastReload2_Implementation()
{
	ArmsMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	IsReloading = false;
	CanAim = true;

	if (GetWeapon(true)->GetFireMode() != 0 && HoldingFireInput == true)
	{
		Fire();
	}

	if (HoldingAimInput == true)
	{
		Aim();
	}
}

void AMultiplayerCharacter::CancelReload(bool PutArmsBackUp)
{
	if (HasAuthority())
	{
		MulticastCancelReload();
	}
	else
	{
		ServerCancelReload();
	}
}

void AMultiplayerCharacter::ServerCancelReload_Implementation(bool PutArmsBackUp)
{
	MulticastCancelReload();
}

void AMultiplayerCharacter::MulticastCancelReload_Implementation(bool PutArmsBackUp)
{
	if (IsReloading == true)
	{
		GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

		if (PutArmsBackUp == true)
		{
			ArmsMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		}

		IsReloading = false;
	}
}

// Called when the game starts or when spawned
void AMultiplayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RecalculateBaseEyeHeight();

	GetOwningController();

	SetupInput();

	SetSensitivity();

	if (HealthComponent)
	{
		GetHealthComponent()->OnDie.AddDynamic(this, &AMultiplayerCharacter::Die);
		GetHealthComponent()->OnDieUnreplicated.AddDynamic(this, &AMultiplayerCharacter::ClientDie);

		if (OwningController)
		{
			HealthComponent->SetOwningPlayerController(OwningController);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "OwningController Invalid MultiplayerCharacter.cpp:BeginPlay");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "HealthComponent Invalid MultiplayerCharacter.cpp:BeginPlay");
	}
}

// Called every frame
void AMultiplayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerCharacter, PlayerIndex);
	DOREPLIFETIME(AMultiplayerCharacter, OwningController);
	DOREPLIFETIME(AMultiplayerCharacter, CanInteract);
	DOREPLIFETIME(AMultiplayerCharacter, CanShoot);
	DOREPLIFETIME(AMultiplayerCharacter, AllWeapons);
	DOREPLIFETIME(AMultiplayerCharacter, CurrentWeaponIndex);
	DOREPLIFETIME(AMultiplayerCharacter, UseADS);
	DOREPLIFETIME(AMultiplayerCharacter, CanAim);
}

// Called to bind functionality to input
void AMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMultiplayerCharacter::Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMultiplayerCharacter::Look);
		EnhancedInputComponent->BindAction(IA_GamepadLook, ETriggerEvent::Triggered, this, &AMultiplayerCharacter::GamepadLook);
		EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &AMultiplayerCharacter::Interact);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AMultiplayerCharacter::PressJump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AMultiplayerCharacter::HoldJump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AMultiplayerCharacter::StopJumping);
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &AMultiplayerCharacter::PressFireInput);
		EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AMultiplayerCharacter::ReleaseFireInput);
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Started, this, &AMultiplayerCharacter::AimInput);
		EnhancedInputComponent->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AMultiplayerCharacter::ReleaseAimInput);
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Started, this, &AMultiplayerCharacter::Reload);
		EnhancedInputComponent->BindAction(IA_SwitchWeapons, ETriggerEvent::Started, this, &AMultiplayerCharacter::SwitchWeaponsInput);
		EnhancedInputComponent->BindAction(IA_GamepadSwitchWeapons, ETriggerEvent::Started, this, &AMultiplayerCharacter::NextWeapon);
	}
}