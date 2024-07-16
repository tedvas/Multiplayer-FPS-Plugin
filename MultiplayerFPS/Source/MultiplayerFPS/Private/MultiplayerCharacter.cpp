// Fill out your copyright notice in the Description page of Project Settings.
// the reason you'll see weird parenthesis on if statements is for linux compatibility

#include "MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "MultiplayerPlayerController.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

// Sets default values
AMultiplayerCharacter::AMultiplayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	/* ************* Components ************* */
	SpringArmParent = RootComponent;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(SpringArmParent);
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 68.0f));
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bUsePawnControlRotation = true;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera Component"));
	CameraComponent->SetupAttachment(SpringArm, NAME_None);
	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms Mesh"));
	ArmsMesh->SetupAttachment(CameraComponent, NAME_None);


	/* ************* Actor Components ************* */
	HealthComponent = CreateDefaultSubobject<UMultiplayerHealthComponent>(TEXT("Health Component"));


	/* ************* Sensitivity ************* */
	FieldOfView = 90.0f;
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
	ToggleAim = false;
	HoldButtonToJump = false;
	UseAimSensitivityMultipler = true;

	IMC_Priority = 0;
	CurrentFOV = 90.0f;
	CanInteract = true;
	InteractDistance = 175.0f;
	OverlappingInteractable = false;
	CanShoot = true;
	ShootingCancelsReload = true;
	CurrentWeaponIndex = 0;
	DropWeaponsOnDeath = 2;
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
	CanGetHitMarkersOnSelf = false;
	HeadSocketName = "head";
	CameraHeadLocation = FVector(2.0f, 7.5f, 0.0f);
	CameraHeadRotation = FRotator(0.0f, 90.0f, -90.0f);
	DelayToDestroyBody = 15.0f;
	TakeDamageControllerVibrationTag = "Take Damage";
	DieControllerVibrationTag = "Die";

	bReplicates = true;
}

void AMultiplayerCharacter::ApplySettings()
{
	SetupInput();
	SetSensitivity();
	SetFOV_BP(FieldOfView);
}

UMultiplayerHealthComponent* AMultiplayerCharacter::GetHealthComponent()
{
	if (HealthComponent)
	{
		return HealthComponent;
	}
	else
	{
		return nullptr;
	}
}

USkeletalMeshComponent* AMultiplayerCharacter::GetPlayerModelMesh_Implementation()
{
	return GetMesh();
}

void AMultiplayerCharacter::SetupInput_Implementation()
{
	ClientSetupInput();
}

void AMultiplayerCharacter::ClientSetupInput_Implementation()
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
	ClientRemoveInput();
}

void AMultiplayerCharacter::ClientRemoveInput_Implementation()
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
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex))
	{
		OwningController = PlayerController;

		if (GetWorldTimerManager().IsTimerActive(GetPlayerControllerTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(GetPlayerControllerTimerHandle);
		}

		if (AMultiplayerPlayerController* MultiplayerController = Cast<AMultiplayerPlayerController>(OwningController))
		{
			PlayerIndex = MultiplayerController->GetPlayerIndex();
			MultiplayerController->ApplySettingsToCharacter();
		}
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerCharacter::GetOwningController);

		if (!GetWorldTimerManager().IsTimerActive(GetPlayerControllerTimerHandle))
		{
			GetWorldTimerManager().SetTimer(GetPlayerControllerTimerHandle, this, &AMultiplayerCharacter::PrintStringForOwningControllerInvalid, 10.0f, false, 10.0f);
		}
	}
}

void AMultiplayerCharacter::PrintStringForOwningControllerInvalid()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Owning Controller Invalid After Trying On Tick For 10 Seconds, Still Retrying MultiplayerCharacter.cpp:GetOwningController_Implementation");
}

void AMultiplayerCharacter::RecalculateBaseEyeHeight()
{
	BaseEyeHeight = CameraComponent->GetRelativeLocation().Z + SpringArm->GetRelativeLocation().Z;
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
	if (CanInteract == true && GetHealthComponent()->GetHealth() > 0)
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
				if (InteractableItemCast->RunInteractOnServer == true)
				{
					ServerInteract(InteractableItemCast, InteractableItemCast->MulticastInteract);
				}

				if (InteractableItemCast->RunInteractOnClient == true)
				{
					ClientInteract(InteractableItemCast);
				}

				if (InteractableItemCast->RunInteractOnServer == false && InteractableItemCast->MulticastInteract == false && InteractableItemCast->RunInteractOnClient == false)
				{
					InteractReplicated(InteractableItemCast);
				}
			}
		}
		else if (OverlappingInteractable == true && InteractableBeingOverlapped)
		{
			AMultiplayerGun* GunCast = Cast<AMultiplayerGun>(InteractableBeingOverlapped);

			if (!GunCast)
			{
				if (InteractableBeingOverlapped->RunInteractOnServer == true)
				{
					ServerInteract(InteractableBeingOverlapped, InteractableBeingOverlapped->MulticastInteract);
				}

				if (InteractableBeingOverlapped->RunInteractOnClient == true)
				{
					ClientInteract(InteractableBeingOverlapped);
				}
			}

			if ((InteractableBeingOverlapped->RunInteractOnServer == false && InteractableBeingOverlapped->MulticastInteract == false && InteractableBeingOverlapped->RunInteractOnClient == false) || GunCast)
			{
				InteractReplicated(InteractableBeingOverlapped);
			}
		}
	}
}

void AMultiplayerCharacter::InteractReplicated(AInteractableItem* Interactable)
{
	if (CanInteract == true && Interactable && GetHealthComponent()->GetHealth() > 0)
	{
		if (AMultiplayerGun* GunCast = Cast<AMultiplayerGun>(Interactable))
		{
			PickupItem(GunCast);
		}
		else
		{
			Interactable->Interact(this);
		}
	}
}

void AMultiplayerCharacter::ServerInteract_Implementation(AInteractableItem* Interactable, bool Multicast)
{
	if (Multicast == true)
	{
		MulticastInteract(Interactable);
	}
	else
	{
		InteractReplicated(Interactable);
	}
}

void AMultiplayerCharacter::MulticastInteract_Implementation(AInteractableItem* Interactable)
{
	InteractReplicated(Interactable);
}

void AMultiplayerCharacter::ClientInteract_Implementation(AInteractableItem* Interactable)
{
	InteractReplicated(Interactable);
}

void AMultiplayerCharacter::PickupItem(AInteractableItem* ItemToPickup)
{
	if (!HasAuthority())
	{
		ServerPickupItem(ItemToPickup);
		return;
	}

	if (ItemToPickup)
	{
		if (AMultiplayerGun* GunCast = Cast<AMultiplayerGun>(ItemToPickup))
		{
			StopFiring(true);

			if (GetAmountOfWeapons() >= GetMaxWeaponAmount())
			{
				if (AMultiplayerGun* Weapon = GetWeapon(true))
				{
					RemoveWeapon(false, false, Weapon);
				}
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

void AMultiplayerCharacter::SetOverlappingInteractable(bool NewOverlappingInteractable)
{
	OverlappingInteractable = NewOverlappingInteractable;
}

bool AMultiplayerCharacter::GetOverlappingInteractable()
{
	return OverlappingInteractable;
}

void AMultiplayerCharacter::SetInteractableBeingOverlapped(AInteractableItem* NewInteractableBeingOverlapped)
{
	InteractableBeingOverlapped = NewInteractableBeingOverlapped;
}

AInteractableItem* AMultiplayerCharacter::GetInteractableBeingOverlapped()
{
	return InteractableBeingOverlapped;
}

void AMultiplayerCharacter::OnTakeDamage(int OldHealth, int NewHealth)
{
	OnTakeDamage_BP(OldHealth, NewHealth);
	ClientOnTakeDamage(OldHealth, NewHealth);
}

void AMultiplayerCharacter::ClientOnTakeDamage_Implementation(int OldHealth, int NewHealth)
{
	if (OldHealth > NewHealth)
	{
		if (OwningController)
		{
			OwningController->ClientStartCameraShake(TakeDamageCameraShake);

			if (TakeDamageControllerVibration)
			{
				if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(OwningController))
				{
					ControllerCast->VibrateController(TakeDamageControllerVibration, TakeDamageControllerVibrationTag);
				}
			}
		}
	}
}

void AMultiplayerCharacter::Die()
{
	StopAiming();

	Die1();
}

void AMultiplayerCharacter::Die1()
{
	if (!HasAuthority())
	{
		ServerDie1();
	}

	if (GetPlayerModelMesh())
	{
		ArmsMesh->SetHiddenInGame(true);
		ArmsMesh->SetVisibility(false);

		SpringArm->bUsePawnControlRotation = false;
		SpringArm->AttachToComponent(GetPlayerModelMesh(), FAttachmentTransformRules::KeepWorldTransform, HeadSocketName);
		SpringArm->SetRelativeLocationAndRotation(CameraHeadLocation, CameraHeadRotation);

		GetPlayerModelMesh()->SetSimulatePhysics(true);
		GetPlayerModelMesh()->SetOwnerNoSee(false);
	}

	if (GetController())
	{
		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(GetController()))
		{
			if (ControllerCast->GetRespawnDelay() > DelayToDestroyBody && DelayToDestroyBody > 0.0f)
			{
				DelayToDestroyBody = ControllerCast->GetRespawnDelay();
			}

			ControllerCast->Respawn(ControllerCast->GetRespawnDelay());
		}
	}

	if (DropWeaponsOnDeath == 2)
	{
		RemoveWeapon(true);
	}
	else if (DropWeaponsOnDeath == 1 && GetWeapon())
	{
		RemoveWeapon(false, false, GetWeapon());
		RemoveWeapon(true, true);
	}
	else
	{
		RemoveWeapon(true, true);
	}

	GetHealthComponent()->DestroyComponent();

	if (DelayToDestroyBody > 0.0f)
	{
		GetWorldTimerManager().SetTimer(DestroySelfTimerHandle, this, &AMultiplayerCharacter::DestroySelf, DelayToDestroyBody, false, DelayToDestroyBody);
	}
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

				if (DieControllerVibration)
				{
					ControllerCast->VibrateController(DieControllerVibration, DieControllerVibrationTag);
				}
			}
		}

		OwningController->ClientStartCameraShake(DieDamageCameraShake);
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

void AMultiplayerCharacter::SetAllSharedCalibers(TMap<FName, int32> NewAllSharedCalibers)
{
	NewAllSharedCalibers.GenerateKeyArray(AllSharedCaliberNames);
	NewAllSharedCalibers.GenerateValueArray(AllSharedCaliberAmounts);
}

TMap<FName, int32> AMultiplayerCharacter::GetAllSharedCalibers()
{
	TMap<FName, int32> Calibers;

	if (AllSharedCaliberNames.Num() > 0)
	{
		for (int32 Index = 0; Index != AllSharedCaliberNames.Num(); ++Index)
		{
			if (AllSharedCaliberNames.IsValidIndex(Index))
			{
				Calibers.Add(AllSharedCaliberNames[Index], AllSharedCaliberAmounts[Index]);
			}
		}
	}

	return Calibers;
}

TArray<FName> AMultiplayerCharacter::GetAllSharedCaliberNames()
{
	TArray<FName> SharedCaliberNames;
	GetAllSharedCalibers().GenerateKeyArray(SharedCaliberNames);

	return SharedCaliberNames;
}

void AMultiplayerCharacter::SetSharedCaliberName(int32 Index, FName NewName)
{
	TMap<FName, int32> NewAllSharedCalibers;
	TArray<FName> AllCaliberNames;
	TArray<int32> AllCaliberAmounts;

	GetAllSharedCalibers().GenerateKeyArray(AllCaliberNames);
	GetAllSharedCalibers().GenerateValueArray(AllCaliberAmounts);

	if (!AllCaliberNames.IsValidIndex(Index))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Index Invalid MultiplayerCharacter.cpp:SetSharedCaliberName");
		return;
	}

	for (int32 i = 0; i != GetAllSharedCalibers().Num(); ++i)
	{
		if (i == Index)
		{
			NewAllSharedCalibers.Add(NewName, AllCaliberAmounts[i]);
		}
		else
		{
			NewAllSharedCalibers.Add(AllCaliberNames[i], AllCaliberAmounts[i]);
		}
	}

	SetAllSharedCalibers(NewAllSharedCalibers);
}

FName AMultiplayerCharacter::GetSharedCaliberName(int32 Index)
{
	TArray<FName> SharedCaliberNames;
	GetAllSharedCalibers().GenerateKeyArray(SharedCaliberNames);

	if (SharedCaliberNames.IsValidIndex(Index))
	{
		return SharedCaliberNames[Index];
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Index Invalid MultiplayerCharacter.cpp:GetSharedCaliberName");
		return "N/A";
	}
}

void AMultiplayerCharacter::SetSharedCaliberAmount(int32 Index, int32 NewAmount)
{
	TMap<FName, int32> NewAllSharedCalibers;
	TArray<FName> AllCaliberNames;
	TArray<int32> AllCaliberAmounts;

	GetAllSharedCalibers().GenerateKeyArray(AllCaliberNames);
	GetAllSharedCalibers().GenerateValueArray(AllCaliberAmounts);

	if (!AllCaliberAmounts.IsValidIndex(Index))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Index Invalid MultiplayerCharacter.cpp:SetSharedCaliberAmount");
		return;
	}

	for (int32 i = 0; i != GetAllSharedCalibers().Num(); ++i)
	{
		if (i == Index)
		{
			NewAllSharedCalibers.Add(AllCaliberNames[i], NewAmount);
		}
		else
		{
			NewAllSharedCalibers.Add(AllCaliberNames[i], AllCaliberAmounts[i]);
		}
	}

	SetAllSharedCalibers(NewAllSharedCalibers);
}

TArray<int32> AMultiplayerCharacter::GetAllSharedCaliberAmounts()
{
	TArray<int32> SharedCaliberAmounts;
	GetAllSharedCalibers().GenerateValueArray(SharedCaliberAmounts);

	return SharedCaliberAmounts;
}

int32 AMultiplayerCharacter::GetSharedCaliberAmount(int32 Index)
{
	TArray<int32> SharedCaliberAmounts;
	GetAllSharedCalibers().GenerateValueArray(SharedCaliberAmounts);

	if (SharedCaliberAmounts.IsValidIndex(Index))
	{
		return SharedCaliberAmounts[Index];
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Index Invalid MultiplayerCharacter.cpp:GetSharedCaliberAmount");
		return -1;
	}
}

AMultiplayerGun* AMultiplayerCharacter::GetWeapon(bool GetCurrentWeapon, int WeaponIndex)
{
	if (GetAmountOfWeapons() > 0)
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
	if (GetAmountOfWeapons() > 0)
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
			return -1;
		}
	}
	else
	{
		return -1;
	}
}

void AMultiplayerCharacter::SetWeaponVisibility(int WeaponVisibilityToChange, bool Visible, bool SetAllOtherWeaponsToOppositeVisibility)
{
	AMultiplayerGun* WeaponToCheck = GetWeapon(false, WeaponVisibilityToChange);

	if (GetAmountOfWeapons() > 0 && AllWeapons.Contains(WeaponToCheck) == true)
	{
		if (SetAllOtherWeaponsToOppositeVisibility == true)
		{
			for (int32 Index = 0; Index != AllWeapons.Num(); ++Index)
			{
				if (Index != WeaponVisibilityToChange)
				{
					if (AMultiplayerGun* Weapon = GetWeapon(false, Index))
					{
						Weapon->SetActorHiddenInGame(Visible);
					}
				}
			}
		}

		if (AMultiplayerGun* Weapon = GetWeapon(false, WeaponVisibilityToChange))
		{
			Weapon->SetActorHiddenInGame(!Visible);
		}
	}
}

void AMultiplayerCharacter::GiveLoadout(TArray<TSubclassOf<AMultiplayerGun>> Loadout, int MaxWeaponAmount)
{
	if (!HasAuthority())
	{
		ServerGiveLoadout(Loadout, MaxWeaponAmount);
		return;
	}

	TArray<TSubclassOf<AMultiplayerGun>> WeaponsToGive = Loadout;

	if (Loadout.Num() <= GetMaxWeaponAmount() && OwningController)
	{
		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(OwningController))
		{
			TArray<TSubclassOf<AMultiplayerGun>> AllPossibleWeapons = ControllerCast->AllWeaponClasses;

			if (ControllerCast->GetRandomizeUnselectedWeapons() == true)
			{
				if ((ControllerCast->GetAvoidDuplicatesForRandomWeapons() == 1 && WeaponsToGive.Num() < AllPossibleWeapons.Num()) || ControllerCast->GetAvoidDuplicatesForRandomWeapons() == 2)
				{
					for (auto& Weapon : WeaponsToGive)
					{
						if (Weapon)
						{
							AllPossibleWeapons.Remove(Weapon);
						}
					}
				}

				for (int32 Index = Loadout.Num(); Index != GetMaxWeaponAmount(); ++Index)
				{
					int RandomWeaponIndex = FMath::RandRange(0, AllPossibleWeapons.Num() - 1);

					if (AllPossibleWeapons.IsValidIndex(RandomWeaponIndex))
					{
						WeaponsToGive.Add(AllPossibleWeapons[RandomWeaponIndex]);

						if (ControllerCast->GetAvoidDuplicatesForRandomWeapons() == 1 && WeaponsToGive.Num() < AllPossibleWeapons.Num())
						{
							AllPossibleWeapons.RemoveAt(RandomWeaponIndex);
						}
						else if (ControllerCast->GetAvoidDuplicatesForRandomWeapons() == 1 && WeaponsToGive.Num() >= AllPossibleWeapons.Num() && GetAmountOfWeapons() < GetMaxWeaponAmount())
						{
							AllPossibleWeapons = ControllerCast->AllWeaponClasses;
						}
						else if (ControllerCast->GetAvoidDuplicatesForRandomWeapons() == 2)
						{
							AllPossibleWeapons.RemoveAt(RandomWeaponIndex);
						}
					}
				}
			}
		}
	}

	if (WeaponsToGive.Num() > MaxWeaponAmount)
	{
		TArray<int32> IndexesToRemove;
		
		for (int32 Index = MaxWeaponAmount; Index != WeaponsToGive.Num(); ++Index)
		{
			if (WeaponsToGive.IsValidIndex(Index) == true)
			{
				IndexesToRemove.Add(Index);
			}
		}

		if (IndexesToRemove.Num() > 0)
		{
			for (auto& Index : IndexesToRemove)
			{
				if (WeaponsToGive.IsValidIndex(Index))
				{
					WeaponsToGive.RemoveAt(Index);
				}
			}
		}
	}

	for (auto& Weapon : WeaponsToGive)
	{
		if (GetAmountOfWeapons() <= MaxWeaponAmount && Weapon)
		{
			GiveWeapon(Weapon, nullptr);
		}
	}

	SwitchWeapons(0);
}

void AMultiplayerCharacter::ServerGiveLoadout_Implementation(const TArray<TSubclassOf<AMultiplayerGun>>& Loadout, int MaxWeaponAmount)
{
	GiveLoadout(Loadout, MaxWeaponAmount);
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

void AMultiplayerCharacter::RemoveWeaponPastIndex(int WeaponIndex, bool DestroyWeapon)
{
	if (WeaponIndex <= 0)
	{
		RemoveWeapon(true, DestroyWeapon);
	}
	else if (GetAmountOfWeapons() + 1 >= WeaponIndex)
	{
		int WeaponAmount = GetAmountOfWeapons();

		for (int32 Index = WeaponIndex; Index != WeaponAmount; ++Index)
		{
			RemoveWeapon(false, DestroyWeapon, nullptr, WeaponIndex);
		}
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

		float WeaponSwitchAnimationTime = 0.9f;

		if (GetWeapon())
		{
			if (GetWeapon()->WeaponSwitchAnimation)
			{
				ArmsMesh->PlayAnimation(GetWeapon()->WeaponSwitchAnimation, false);

				if (GetWeapon()->WeaponSwitchTime > 0.0f)
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchTime;
				}
				else
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchAnimation->GetPlayLength();
				}
			}

			if (GetWeapon()->SwitchOffGunAnimation && GetWeapon()->UseSkeletalMesh == true && GetWeapon()->GunSkeletalMesh)
			{
				GetWeapon()->GunSkeletalMesh->PlayAnimation(GetWeapon()->SwitchOffGunAnimation, false);
			}
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
			GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, SwitchWeaponsTimerDelegate, WeaponSwitchAnimationTime, false);
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

		float WeaponSwitchAnimationTime = 0.9f;

		if (GetWeapon())
		{
			if (GetWeapon()->WeaponSwitchAnimation)
			{
				ArmsMesh->PlayAnimation(GetWeapon()->WeaponSwitchAnimation, false);

				if (GetWeapon()->WeaponSwitchTime > 0.0f)
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchTime;
				}
				else
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchAnimation->GetPlayLength();
				}
			}

			if (GetWeapon()->SwitchOffGunAnimation && GetWeapon()->UseSkeletalMesh == true && GetWeapon()->GunSkeletalMesh)
			{
				GetWeapon()->GunSkeletalMesh->PlayAnimation(GetWeapon()->SwitchOffGunAnimation, false);
			}
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
			GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, SwitchWeaponsTimerDelegate, WeaponSwitchAnimationTime, false);
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
	if (GetAmountOfWeapons() > 0)
	{
		SetCanShoot(false);
		SetCanAim(false);
		StopFiring(true);
		StopAiming();
		CancelReload(false);

		float WeaponSwitchAnimationTime = 0.9f;

		if (GetWeapon())
		{
			if (GetWeapon()->UseTwoWeaponSwitchAnimations == true && GetWeapon()->WeaponSwitchAnimation1)
			{
				ArmsMesh->PlayAnimation(GetWeapon()->WeaponSwitchAnimation1, false);

				if (GetWeapon()->WeaponSwitchTime > 0.0f)
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchTime;
				}
				else
				{
					WeaponSwitchAnimationTime = GetWeapon()->WeaponSwitchAnimation1->GetPlayLength();
				}
			}

			if (GetWeapon()->SwitchToGunAnimation && GetWeapon()->UseSkeletalMesh == true && GetWeapon()->GunSkeletalMesh)
			{
				GetWeapon()->GunSkeletalMesh->PlayAnimation(GetWeapon()->SwitchToGunAnimation, false);
			}
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
			UseActorClassesForHitMarkers = Gun->GetUseActorClassesForHitMarkers();
			HitMarkerActorSounds = Gun->GetHitMarkerActorSounds();
			HitMarkerSurfaceSounds = Gun->GetHitMarkerSurfaceSounds();
		}

		SetWeaponVisibility(CurrentWeaponIndex, true);

		GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerCharacter::SwitchWeapons1, WeaponSwitchAnimationTime, false, WeaponSwitchAnimationTime);
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

	if (GetAmountOfWeapons() > 0)
	{
		SetArmsAnimationMode();

		SetCanShoot(true);
		SetCanAim(true);

		if (AMultiplayerGun* Weapon = GetWeapon(true))
		{
			if (Weapon->GetAmmoInMagazine() <= 0 && Weapon->GetInfiniteAmmo() != 2)
			{
				Reload();
			}
			else if (Weapon->GetFireMode() != 0 && HoldingFireInput == true)
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
	if (GetAmountOfWeapons() > 0)
	{
		if (GetCanShoot() == true)
		{
			if (AMultiplayerGun* Weapon = GetWeapon(true))
			{
				if (Weapon->GetAmmoInMagazine() > 0 || Weapon->GetInfiniteAmmo() == 2)
				{
					if (ShootingCancelsReload == true || IsReloading == false)
					{
						CancelReload();

						Weapon->FireInput();
					}
				}
			}
		}
	}
}

void AMultiplayerCharacter::StopFiring(bool EvenCancelBurst)
{
	for (auto& Weapon : GetAllWeapons())
	{
		if (Weapon)
		{
			Weapon->StopFiring(EvenCancelBurst);
		}
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
	if (CanAim == true && IsAiming == false)
	{
		if (AMultiplayerGun* CurrentWeapon = GetWeapon(true))
		{
			if (AimingCancelsReload == true || IsReloading == false)
			{
				FVector AimLocation;
				FRotator AimRotation;

				if ((UseADS == 0 && CurrentWeapon->GetUseADS() == 0) || CurrentWeapon->GetUseADS() == 2 || (UseADS == 1 && CurrentWeapon->GetUseADS() < 2))
				{
					AimLocation = CurrentWeapon->GetADSArmsLocation();
					AimRotation = CurrentWeapon->GetADSArmsRotation();

					if (CurrentWeapon->GetDivideAimingFOV() == true)
					{
						SetAimingFOV_BP(true, FieldOfView / CurrentWeapon->GetADSFOV(), CurrentWeapon->GetTimeToADS());
					}
					else
					{
						SetAimingFOV_BP(true, FieldOfView - CurrentWeapon->GetADSFOV(), CurrentWeapon->GetTimeToADS());
					}

					FLatentActionInfo LatentActionInfo;
					LatentActionInfo.CallbackTarget = this;

					UKismetSystemLibrary::MoveComponentTo(ArmsMesh, AimLocation, AimRotation, false, false, CurrentWeapon->GetTimeToADS(), false, EMoveComponentAction::Move, LatentActionInfo);
				}
				else
				{
					AimLocation = CurrentWeapon->GetZoomArmsLocation();
					AimRotation = CurrentWeapon->GetZoomArmsRotation();

					if (CurrentWeapon->GetDivideAimingFOV() == true)
					{
						SetAimingFOV_BP(true, FieldOfView / CurrentWeapon->GetZoomFOV(), CurrentWeapon->GetTimeToZoom());
					}
					else
					{
						SetAimingFOV_BP(true, FieldOfView - CurrentWeapon->GetZoomFOV(), CurrentWeapon->GetTimeToZoom());
					}

					FLatentActionInfo LatentActionInfo;
					LatentActionInfo.CallbackTarget = this;

					UKismetSystemLibrary::MoveComponentTo(ArmsMesh, AimLocation, AimRotation, false, false, CurrentWeapon->GetTimeToZoom(), false, EMoveComponentAction::Move, LatentActionInfo);
				}
			}
		}
	}

	if (HasAuthority())
	{
		MulticastAim();
	}
	else
	{
		ServerAim();
	}
}

void AMultiplayerCharacter::ServerAim_Implementation()
{
	MulticastAim();
}

void AMultiplayerCharacter::MulticastAim_Implementation()
{
	if (CanAim == true && IsAiming == false)
	{
		if (AMultiplayerGun* CurrentWeapon = GetWeapon(true))
		{
			if (AimingCancelsReload == true || IsReloading == false)
			{
				CancelReload();

				IsAiming = true;

				SetSensitivity();

				FVector AimLocation;
				FRotator AimRotation;

				if ((UseADS == 0 && CurrentWeapon->GetUseADS() == 0) || CurrentWeapon->GetUseADS() == 2 || (UseADS == 1 && CurrentWeapon->GetUseADS() < 2))
				{
					IsADSing = true;
				}
				else
				{
					IsZoomedIn = true;
				}
			}
		}
	}
}

void AMultiplayerCharacter::StopAiming()
{
	if (AMultiplayerGun* CurrentWeapon = GetWeapon(true))
	{
		if (IsAiming == true || IsADSing == true || IsZoomedIn == true)
		{
			float AimTime;

			if ((UseADS == 0 && CurrentWeapon->GetUseADS() == 0) || CurrentWeapon->GetUseADS() == 2 || (UseADS == 1 && CurrentWeapon->GetUseADS() < 2))
			{
				AimTime = CurrentWeapon->GetTimeToADS();

				if (CurrentWeapon->GetDivideAimingFOV() == true)
				{
					SetAimingFOV_BP(false, FieldOfView / CurrentWeapon->GetADSFOV(), AimTime);
				}
				else
				{
					SetAimingFOV_BP(false, FieldOfView - CurrentWeapon->GetADSFOV(), AimTime);
				}
			}
			else
			{
				AimTime = CurrentWeapon->GetTimeToZoom();

				if (CurrentWeapon->GetDivideAimingFOV() == true)
				{
					SetAimingFOV_BP(false, FieldOfView / CurrentWeapon->GetZoomFOV(), AimTime);
				}
				else
				{
					SetAimingFOV_BP(false, FieldOfView - CurrentWeapon->GetZoomFOV(), AimTime);
				}
			}

			FLatentActionInfo LatentActionInfo;
			LatentActionInfo.CallbackTarget = this;

			UKismetSystemLibrary::MoveComponentTo(ArmsMesh, CurrentWeapon->GetPlayerArmsRelativeLocation(), CurrentWeapon->GetPlayerArmsRelativeRotation(), false, false, AimTime, false, EMoveComponentAction::Move, LatentActionInfo);
		}
	}

	if (HasAuthority())
	{
		MulticastStopAiming();
	}
	else
	{
		ServerStopAiming();
	}
}

void AMultiplayerCharacter::ServerStopAiming_Implementation()
{
	MulticastStopAiming();
}

void AMultiplayerCharacter::MulticastStopAiming_Implementation()
{
	if (AMultiplayerGun* CurrentWeapon = GetWeapon(true))
	{
		if (IsAiming == true || IsADSing == true || IsZoomedIn == true)
		{
			IsAiming = false;
			IsADSing = false;
			IsZoomedIn = false;

			SetSensitivity();
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

void AMultiplayerCharacter::ServerReload_Implementation()
{
	MulticastReload();
}

void AMultiplayerCharacter::MulticastReload_Implementation()
{
	if (GetAmountOfWeapons() > 0 && CanReload == true && IsReloading == false)
	{
		if (AMultiplayerGun* Weapon = GetWeapon(true))
		{
			if (Weapon->GetAmmoInMagazine() < Weapon->GetMaxAmmoInMagazine() && Weapon->GetInfiniteAmmo() != 2)
			{
				if ((Weapon->GetUseSharedCalibers() == false && Weapon->GetReserveAmmo() > 0) || (Weapon->GetUseSharedCalibers() == true && GetSharedCaliberAmount(Weapon->GetCaliberToUse()) > 0) || Weapon->GetInfiniteAmmo() == 1)
				{
					StopFiring(true);
					StopAiming();

					if (Weapon->GetAmmoInMagazine() <= 0)
					{
						CanAim = false;
					}

					IsReloading = true;

					if (Weapon->ReloadAnimation && Weapon->GetAmmoInMagazine() > 0)
					{
						ArmsMesh->PlayAnimation(Weapon->ReloadAnimation, false);
					}
					else if (Weapon->ReloadEmptyAnimation && Weapon->GetAmmoInMagazine() <= 0)
					{
						ArmsMesh->PlayAnimation(Weapon->ReloadEmptyAnimation, false);
					}

					if (Weapon->UseSkeletalMesh == true && Weapon->GunSkeletalMesh)
					{
						if (Weapon->GetAmmoInMagazine() > 0 && Weapon->ReloadGunAnimation)
						{
							Weapon->GunSkeletalMesh->PlayAnimation(Weapon->ReloadGunAnimation, false);
						}
						else if (Weapon->GetAmmoInMagazine() <= 0 && Weapon->ReloadEmptyGunAnimation)
						{
							Weapon->GunSkeletalMesh->PlayAnimation(Weapon->ReloadEmptyGunAnimation, false);
						}
					}

					if (GetPlayerModelMesh())
					{
						if (Weapon->ThirdPersonReloadAnimation && Weapon->GetAmmoInMagazine() > 0)
						{
							GetPlayerModelMesh()->PlayAnimation(Weapon->ThirdPersonReloadAnimation, false);
						}
						else if (Weapon->ThirdPersonReloadEmptyAnimation && Weapon->GetAmmoInMagazine() <= 0)
						{
							GetPlayerModelMesh()->PlayAnimation(Weapon->ThirdPersonReloadEmptyAnimation, false);
						}
					}

					if (ReloadCharacterSound)
					{
						UGameplayStatics::SpawnSoundAttached(ReloadCharacterSound, ArmsMesh, Weapon->GetSocketName(), FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true);
					}

					if (Weapon->ReloadSpeed > 0)
					{
						GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerCharacter::Reload1, Weapon->ReloadSpeed, false, Weapon->ReloadSpeed);
					}
					else
					{
						Reload1();
					}
				}
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
	AMultiplayerGun* Weapon = GetWeapon(true);

	IsReloading = false;
	CanAim = true;

	if (Weapon)
	{
		int AmmoInMagazine = Weapon->GetAmmoInMagazine();

		if (Weapon->UseTwoReloadAnimations == true)
		{
			if (Weapon->ReloadAnimation1 && Weapon->GetAmmoInMagazine() > 0)
			{
				ArmsMesh->PlayAnimation(Weapon->ReloadAnimation1, false);
			}
			else if (Weapon->ReloadEmptyAnimation1 && Weapon->GetAmmoInMagazine() <= 0)
			{
				ArmsMesh->PlayAnimation(Weapon->ReloadEmptyAnimation1, false);
			}

			if (GetPlayerModelMesh())
			{
				if (Weapon->ThirdPersonReloadAnimation1 && Weapon->GetAmmoInMagazine() > 0)
				{
					GetPlayerModelMesh()->PlayAnimation(Weapon->ThirdPersonReloadAnimation1, false);
				}
				else if (Weapon->ThirdPersonReloadEmptyAnimation1 && Weapon->GetAmmoInMagazine() <= 0)
				{
					GetPlayerModelMesh()->PlayAnimation(Weapon->ThirdPersonReloadEmptyAnimation1, false);
				}
			}
		}

		Weapon->Reload();

		bool StartTimer = false;
		float TimerLength = 0.0f;

		if (Weapon->ReloadSpeed1 > 0)
		{
			StartTimer = true;
			TimerLength = Weapon->ReloadSpeed1;
		}
		else if (Weapon->ReloadSpeed1 >= -1 && Weapon->ReloadSpeed1 < 0)
		{
			if (AmmoInMagazine > 0)
			{
				if (Weapon->UseTwoReloadAnimations == true && Weapon->ReloadAnimation1)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadAnimation1->GetPlayLength();
				}
				else if (Weapon->UseTwoReloadAnimations == false && Weapon->ReloadAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadAnimation->GetPlayLength();
				}
			}
			else
			{
				if (Weapon->UseTwoReloadAnimations == true && Weapon->ReloadEmptyAnimation1)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadEmptyAnimation1->GetPlayLength();
				}
				else if (Weapon->UseTwoReloadAnimations == false && Weapon->ReloadEmptyAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadEmptyAnimation->GetPlayLength();
				}
			}
		}
		else if (Weapon->ReloadSpeed1 >= -2 && Weapon->ReloadSpeed1 < -1)
		{
			if (AmmoInMagazine > 0)
			{
				if (Weapon->ReloadGunAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadGunAnimation->GetPlayLength();
				}
			}
			else
			{
				if (Weapon->ReloadEmptyGunAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ReloadEmptyGunAnimation->GetPlayLength();
				}
			}
		}
		else if (Weapon->ReloadSpeed1 >= -3 && Weapon->ReloadSpeed1 < -2)
		{
			if (AmmoInMagazine > 0)
			{
				if (Weapon->UseTwoReloadAnimations == true && Weapon->ThirdPersonReloadAnimation1)
				{
					StartTimer = true;
					TimerLength = Weapon->ThirdPersonReloadAnimation1->GetPlayLength();
				}
				else if (Weapon->UseTwoReloadAnimations == false && Weapon->ThirdPersonReloadAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ThirdPersonReloadAnimation->GetPlayLength();
				}
			}
			else
			{
				if (Weapon->UseTwoReloadAnimations == true && Weapon->ThirdPersonReloadEmptyAnimation1)
				{
					StartTimer = true;
					TimerLength = Weapon->ThirdPersonReloadEmptyAnimation1->GetPlayLength();
				}
				else if (Weapon->UseTwoReloadAnimations == false && Weapon->ThirdPersonReloadEmptyAnimation)
				{
					StartTimer = true;
					TimerLength = Weapon->ThirdPersonReloadEmptyAnimation->GetPlayLength();
				}
			}
		}

		if (StartTimer == true && TimerLength > 0.0f)
		{
			GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerCharacter::Reload2, TimerLength, false, TimerLength);
		}
		else
		{
			Reload2();
		}
	}
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
	SetArmsAnimationMode();

	if (GetPlayerModelMesh())
	{
		GetPlayerModelMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	IsReloading = false;
	CanAim = true;

	if (AMultiplayerGun* Weapon = GetWeapon(true))
	{
		if (Weapon->GunSkeletalMesh && Weapon->UseSkeletalMesh == true)
		{
			Weapon->GunSkeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		}

		if (Weapon->GetFireMode() != 0 && HoldingFireInput == true)
		{
			Fire();
		}
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
			SetArmsAnimationMode();

			if (GetPlayerModelMesh())
			{
				GetPlayerModelMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			}

			if (AMultiplayerGun* Weapon = GetWeapon(true))
			{
				if (Weapon->GunSkeletalMesh && Weapon->UseSkeletalMesh == true)
				{
					Weapon->GunSkeletalMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				}
			}
		}

		IsReloading = false;
	}
}

void AMultiplayerCharacter::ShowHitMarker_Implementation(AActor* HitActor, UPhysicalMaterial* HitPhysicalMaterial)
{
	if (GetWeapon())
	{
		if (GetWeapon()->GetIsExplosive())
		{
			UseActorClassesForHitMarkers = 0;
		}
	}

	if (UseActorClassesForHitMarkers == 0 && HitActor)
	{
		if ((HitActor != this && CanGetHitMarkersOnSelf == false) || CanGetHitMarkersOnSelf == true)
		{
			ShowHitMarker_BP(HitActor, HitPhysicalMaterial);

			TArray<TSubclassOf<AActor>> Actors;
			TArray<USoundBase*> HitMarkerSounds;

			HitMarkerActorSounds.GenerateKeyArray(Actors);
			HitMarkerActorSounds.GenerateValueArray(HitMarkerSounds);

			if (Actors.Contains(HitActor->GetClass()))
			{
				int SoundIndex = Actors.Find(HitActor->GetClass());

				if (HitMarkerSounds.IsValidIndex(SoundIndex))
				{
					USoundBase* HitMarkerSound = HitMarkerSounds[SoundIndex];

					if (HitMarkerSound)
					{
						UGameplayStatics::PlaySound2D(GetWorld(), HitMarkerSound);
					}
				}
			}
		}
	}
	else if (UseActorClassesForHitMarkers == 1 && HitPhysicalMaterial)
	{
		ShowHitMarker_BP(HitActor, HitPhysicalMaterial);

		TArray<UPhysicalMaterial*> HitPhysicalMaterials;
		TArray<USoundBase*> HitMarkerSounds;

		HitMarkerSurfaceSounds.GenerateKeyArray(HitPhysicalMaterials);
		HitMarkerSurfaceSounds.GenerateValueArray(HitMarkerSounds);

		if (HitPhysicalMaterials.Contains(HitPhysicalMaterial))
		{
			int SoundIndex = HitPhysicalMaterials.Find(HitPhysicalMaterial);

			if (HitMarkerSounds.IsValidIndex(SoundIndex))
			{
				USoundBase* HitMarkerSound = HitMarkerSounds[SoundIndex];

				if (HitMarkerSound)
				{
					UGameplayStatics::PlaySound2D(GetWorld(), HitMarkerSound);
				}
			}
		}
	}
	else if (UseActorClassesForHitMarkers == 2 && HitActor && HitPhysicalMaterial)
	{
		ShowHitMarker_BP(HitActor, HitPhysicalMaterial);

		TArray<UPhysicalMaterial*> HitPhysicalMaterials;
		TArray<USoundBase*> HitMarkerSounds;

		HitMarkerSurfaceSounds.GenerateKeyArray(HitPhysicalMaterials);
		HitMarkerSurfaceSounds.GenerateValueArray(HitMarkerSounds);

		if (HitPhysicalMaterials.Contains(HitPhysicalMaterial))
		{
			int SoundIndex = HitPhysicalMaterials.Find(HitPhysicalMaterial);

			if (HitMarkerSounds.IsValidIndex(SoundIndex))
			{
				USoundBase* HitMarkerSound = HitMarkerSounds[SoundIndex];

				if (HitMarkerSound)
				{
					UGameplayStatics::PlaySound2D(GetWorld(), HitMarkerSound);
				}
			}
		}
		else
		{
			ShowHitMarker_BP(HitActor, HitPhysicalMaterial);

			TArray<TSubclassOf<AActor>> Actors;

			HitMarkerActorSounds.GenerateKeyArray(Actors);
			HitMarkerActorSounds.GenerateValueArray(HitMarkerSounds);

			if (Actors.Contains(HitActor->GetClass()))
			{
				int SoundIndex = Actors.Find(HitActor->GetClass());

				if (HitMarkerSounds.IsValidIndex(SoundIndex))
				{
					USoundBase* HitMarkerSound = HitMarkerSounds[SoundIndex];

					if (HitMarkerSound)
					{
						UGameplayStatics::PlaySound2D(GetWorld(), HitMarkerSound);
					}
				}
			}
		}
	}
}

void AMultiplayerCharacter::SetArmsAnimationMode(float Delay)
{
	if (Delay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(ArmsAnimationModeTimerHandle, this, &AMultiplayerCharacter::SetArmsAnimationMode1, Delay, false, Delay);
	}
	else
	{
		SetArmsAnimationMode1();
	}
}

void AMultiplayerCharacter::SetArmsAnimationMode1()
{
	if (ArmsMesh)
	{
		ArmsMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}
}

// Called when the game starts or when spawned
void AMultiplayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RecalculateBaseEyeHeight();
	GetOwningController();

	if (HealthComponent)
	{
		GetHealthComponent()->OnTakeDamage.AddDynamic(this, &AMultiplayerCharacter::OnTakeDamage);
		GetHealthComponent()->OnDie.AddDynamic(this, &AMultiplayerCharacter::Die);
		GetHealthComponent()->OnDieUnreplicated.AddDynamic(this, &AMultiplayerCharacter::ClientDie);

		if (OwningController)
		{
			HealthComponent->SetOwningPlayerController(OwningController);
		}
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
	DOREPLIFETIME(AMultiplayerCharacter, IsAiming);
	DOREPLIFETIME(AMultiplayerCharacter, IsADSing);
	DOREPLIFETIME(AMultiplayerCharacter, IsZoomedIn);
	DOREPLIFETIME(AMultiplayerCharacter, AllSharedCaliberNames);
	DOREPLIFETIME(AMultiplayerCharacter, AllSharedCaliberAmounts);
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