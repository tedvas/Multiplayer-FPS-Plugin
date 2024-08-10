// Fill out your copyright notice in the Description page of Project Settings.
// the reason you'll see weird parenthesis on if statements is for linux compatibility

#include "MultiplayerPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerCharacter.h"
#include "MultiplayerGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine.h"

AMultiplayerPlayerController::AMultiplayerPlayerController()
{
	FieldOfView = 90.0f;
	MouseDefaultSensitivityX = 0.5f;
	MouseAimingSensitivityX = 0.275f;
	MouseDefaultSensitivityY = 0.5f;
	MouseAimingSensitivityY = 0.275f;
	MouseAimingSensitivityMultiplier = 0.55f;
	GamepadDefaultSensitivityX = 0.5f;
	GamepadAimingSensitivityX = 0.275f;
	GamepadDefaultSensitivityY = 0.5f;
	GamepadAimingSensitivityY = 0.275f;
	GamepadAimingSensitivityMultiplier = 0.55f;
	ControllerVibration = true;
	ToggleAim = false;
	HoldButtonToJump = false;
	UseAimSensitivityMultipler = true;

	PlayerIndex = 0;
	UsingThirdPerson = false;
	CanSwitchPerspective = true;
	UsingGamepad = false;
	GiveLoadoutOnBeginPlay = true;
	MaxWeaponAmount = 2;
	RandomizeUnselectedWeapons = true;
	AvoidDuplicatesForRandomWeapons = 2;
	RespawnDelay = 2.5f;
	ShowHUDOnRespawn = true;
	RemoveAllWidgetsOnRespawn = false;

	bReplicates = true;
}

APawn* AMultiplayerPlayerController::GetControlledPawn()
{
	return K2_GetPawn();
}

void AMultiplayerPlayerController::CreateUIWidget_Implementation(TSubclassOf<UUserWidget> NewWidget, int32 ZOrder, bool RemoveAllWidgetsFirst, UUserWidget* WidgetToRemove, bool ChangeCursor, bool ShowCursor, bool ChangeInput, bool GameInput, bool ChangePaused, bool PauseGame)
{
	if (NewWidget)
	{
		if (RemoveAllWidgetsFirst == true)
		{
			for (auto& UI : AllUI)
			{
				if (UI)
				{
					UI->RemoveFromParent();
				}
			}

			AllUI.Empty();
			HUD = nullptr;
			DeathScreen = nullptr;
		}
		else if (WidgetToRemove)
		{
			WidgetToRemove->RemoveFromParent();

			AllUI.Remove(WidgetToRemove);

			if (WidgetToRemove->GetClass() == HUDClass)
			{
				HUD = nullptr;
			}

			if (WidgetToRemove->GetClass() == DeathScreenClass)
			{
				DeathScreen = nullptr;
			}
		}

		if (NewWidget == HUDClass && HUD)
		{
			HUD->RemoveFromParent();
			HUD = nullptr;
		}

		if (NewWidget == DeathScreenClass && DeathScreen)
		{
			DeathScreen->RemoveFromParent();
			DeathScreen = nullptr;
		}

		UUserWidget* CreatedWidget = CreateWidget(this, NewWidget);

		if (CreatedWidget)
		{
			CreatedWidget->AddToPlayerScreen(ZOrder);
			AllUI.Add(CreatedWidget);

			if (NewWidget == HUDClass)
			{
				HUD = CreatedWidget;
			}

			if (NewWidget == DeathScreenClass)
			{
				DeathScreen = CreatedWidget;
			}
		}

		if (ChangeCursor == true)
		{
			SetShowMouseCursor(ShowCursor);
		}

		if (ChangeInput == true)
		{
			if (GameInput == true)
			{
				UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
			}
			else
			{
				UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, GetUILastIndex(), EMouseLockMode::DoNotLock, true);
			}
		}

		if (ChangePaused == true)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), PauseGame);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Please Assign NewWidget For CreateUIWidget Function To Execute");
	}
}

void AMultiplayerPlayerController::RemoveUIWidget_Implementation(UUserWidget* WidgetToRemove, bool RemoveAllWidgets, bool ChangeCursor, bool ShowCursor, bool ChangeInput, bool GameInput, bool ChangePaused, bool PauseGame)
{
	if (RemoveAllWidgets == true)
	{
		for (auto& UI : AllUI)
		{
			if (UI)
			{
				UI->RemoveFromParent();
			}
		}

		AllUI.Empty();
		HUD = nullptr;
		DeathScreen = nullptr;
	}
	else if (WidgetToRemove)
	{
		WidgetToRemove->RemoveFromParent();

		AllUI.Remove(WidgetToRemove);

		if (WidgetToRemove->GetClass() == HUDClass)
		{
			HUD = nullptr;
		}

		if (WidgetToRemove->GetClass() == DeathScreenClass)
		{
			DeathScreen = nullptr;
		}
	}

	if (ChangeCursor == true)
	{
		SetShowMouseCursor(ShowCursor);
	}

	if (ChangeInput == true)
	{
		if (GameInput == true)
		{
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
		}
		else
		{
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, GetUILastIndex(), EMouseLockMode::DoNotLock, true);
		}
	}

	if (ChangePaused == true)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), PauseGame);
	}
}

UUserWidget* AMultiplayerPlayerController::GetUILastIndex()
{
	if (AllUI.IsValidIndex(AllUI.Num() - 1) && AllUI.Num() >= 1)
	{
		return AllUI[AllUI.Num() - 1];
	}
	else
	{
		return nullptr;
	}
}

void AMultiplayerPlayerController::PossessPawn(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation, bool KeepVelocity, bool ChangeStartingVelocity, FVector NewVelocity)
{
	if (!HasAuthority())
	{
		ServerPossessPawn(NewPawnToSpawn, NewPawn, SpawnNewPawn, Location, Rotation, DestroyOldPawn, KeepControlRotation, KeepVelocity, ChangeStartingVelocity, NewVelocity);
		return;
	}

	if ((SpawnNewPawn == true && NewPawnToSpawn) || (SpawnNewPawn == false && NewPawn))
	{
		FRotator CurrentControlRotation = GetControlRotation();
		FVector CurrentVeloctiy = FVector(0.0f, 0.0f, 0.0f);

		if (KeepVelocity == true)
		{
			if (ACharacter* CurrentCharacterCast = Cast<ACharacter>(GetControlledPawn()))
			{
				CurrentVeloctiy = CurrentCharacterCast->GetCharacterMovement()->Velocity;
			}
		}

		if (GetWorld()->GetAuthGameMode())
		{
			if (AMultiplayerGameMode* GameModeCast = Cast<AMultiplayerGameMode>(GetWorld()->GetAuthGameMode()))
			{
				GameModeCast->RemovePlayerPawn(GetPawn());
			}
		}

		if (DestroyOldPawn == true && GetPawn())
		{
			GetPawn()->Destroy();
		}
		else
		{
			UnPossess();
		}

		APawn* NewPawnToPossess = nullptr;

		if (SpawnNewPawn == true)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			NewPawnToPossess = GetWorld()->SpawnActor<APawn>(NewPawnToSpawn, Location, Rotation, SpawnParams);
		}
		else
		{
			NewPawnToPossess = NewPawn;
		}

		if (NewPawnToPossess)
		{
			Possess(NewPawnToPossess);

			if (KeepControlRotation == true)
			{
				ClientSetControlRotation(CurrentControlRotation);
			}
			else
			{
				ClientSetControlRotation(Rotation);
			}

			AMultiplayerCharacter* NewPlayerCharacterCast = Cast<AMultiplayerCharacter>(NewPawnToPossess);

			if (SpawnNewPawn == true && NewPlayerCharacterCast)
			{
				NewPlayerCharacterCast->SetAllSharedCalibers(AllSharedCalibersOnSpawn);
			}

			if (KeepVelocity == true || ChangeStartingVelocity == true)
			{
				if (NewPlayerCharacterCast)
				{
					if (NewPlayerCharacterCast->GetCharacterMovement())
					{
						NewPlayerCharacterCast->PlayerIndex = PlayerIndex;

						if (KeepVelocity == true)
						{
							NewPlayerCharacterCast->GetCharacterMovement()->Velocity = CurrentVeloctiy;
						}
						else
						{
							NewPlayerCharacterCast->GetCharacterMovement()->Velocity = NewVelocity;
						}
					}
				}
				else if (ACharacter* NewCharacterCast = Cast<ACharacter>(NewPawnToPossess))
				{
					if (NewCharacterCast->GetCharacterMovement())
					{
						if (KeepVelocity == true)
						{
							NewCharacterCast->GetCharacterMovement()->Velocity = CurrentVeloctiy;
						}
						else
						{
							NewCharacterCast->GetCharacterMovement()->Velocity = NewVelocity;
						}
					}
				}
			}

			if (GetWorld()->GetAuthGameMode())
			{
				if (AMultiplayerGameMode* GameModeCast = Cast<AMultiplayerGameMode>(GetWorld()->GetAuthGameMode()))
				{
					GameModeCast->AddPlayerPawn(NewPawnToPossess);
				}
			}

			if (GiveLoadoutOnBeginPlay == true)
			{
				GiveLoadout();
			}
		}
	}
}

void AMultiplayerPlayerController::ServerPossessPawn_Implementation(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation, bool KeepVelocity, bool ChangeStartingVelocity, FVector NewVelocity)
{
	PossessPawn(NewPawnToSpawn, NewPawn, SpawnNewPawn, Location, Rotation, DestroyOldPawn, KeepControlRotation, KeepVelocity, ChangeStartingVelocity, NewVelocity);
}

void AMultiplayerPlayerController::ApplySettingsToCharacter()
{
	if (GetControlledPawn())
	{
		if (GetWorldTimerManager().IsTimerActive(GetControlledPawnTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(GetControlledPawnTimerHandle);
		}

		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(GetControlledPawn()))
		{
			PlayerCast->SetUsingThirdPerson(GetUsingThirdPerson(), true);
			PlayerCast->FieldOfView = FieldOfView;
			PlayerCast->MouseDefaultSensitivityX = MouseDefaultSensitivityX;
			PlayerCast->MouseAimingSensitivityX = MouseAimingSensitivityX;
			PlayerCast->MouseDefaultSensitivityY = MouseDefaultSensitivityY;
			PlayerCast->MouseAimingSensitivityY = MouseAimingSensitivityY;
			PlayerCast->MouseAimingSensitivityMultiplier = MouseAimingSensitivityMultiplier;
			PlayerCast->GamepadDefaultSensitivityX = GamepadDefaultSensitivityX;
			PlayerCast->GamepadAimingSensitivityX = GamepadAimingSensitivityX;
			PlayerCast->GamepadDefaultSensitivityY = GamepadDefaultSensitivityY;
			PlayerCast->GamepadAimingSensitivityY = GamepadAimingSensitivityY;
			PlayerCast->GamepadAimingSensitivityMultiplier = GamepadAimingSensitivityMultiplier;
			PlayerCast->ToggleAim = ToggleAim;
			PlayerCast->HoldButtonToJump = HoldButtonToJump;
			PlayerCast->UseAimSensitivityMultipler = UseAimSensitivityMultipler;
			PlayerCast->ApplySettings();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Player Cast Failed MultiplayerPlayerController.cpp:ApplySettingsToCharacter");
		}
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerPlayerController::ApplySettingsToCharacter);

		if (!GetWorldTimerManager().IsTimerActive(GetControlledPawnTimerHandle))
		{
			GetWorldTimerManager().SetTimer(GetControlledPawnTimerHandle, this, &AMultiplayerPlayerController::PrintStringForControlledPawnInvalidApplySettings, 10.0f, false, 10.0f);
		}
	}
}

void AMultiplayerPlayerController::PrintStringForControlledPawnInvalidApplySettings()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Controlled Pawn Invalid After Trying On Tick For 10 Seconds, Still Retrying MultiplayerPlayerController.cpp:ApplySettingsToCharacter");
}

void AMultiplayerPlayerController::ClientSetControlRotation_Implementation(FRotator Rotation)
{
	SetControlRotation(Rotation);
}

void AMultiplayerPlayerController::GiveLoadout_Implementation()
{
	if (GetControlledPawn())
	{
		if (AMultiplayerCharacter* NewCharacterCast = Cast<AMultiplayerCharacter>(GetControlledPawn()))
		{
			NewCharacterCast->GiveLoadout(WeaponChoices, MaxWeaponAmount);
		}
	}
}

void AMultiplayerPlayerController::Respawn(float DelayToRespawn)
{
	if (DelayToRespawn > 0)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMultiplayerPlayerController::Respawn1, DelayToRespawn, false, DelayToRespawn);
	}
	else
	{
		Respawn1();
	}
}

void AMultiplayerPlayerController::Respawn1()
{
	if (!HasAuthority())
	{
		ServerRespawn1();
	}

	if (GetControlledPawn())
	{
		TArray<FVector> SpawnLocations;
		TArray<FRotator> SpawnRotations;

		RespawnPoints.GenerateKeyArray(SpawnLocations);
		RespawnPoints.GenerateValueArray(SpawnRotations);

		if (SpawnLocations.Num() >= 1)
		{
			int ChosenIndex;
			FVector Location;
			FRotator Rotation;
			bool SetRotation = true;

			ChosenIndex = FMath::RandRange(0, SpawnLocations.Num() - 1);
			Location = SpawnLocations[ChosenIndex];

			if (SpawnRotations.Num() >= 1)
			{
				Rotation = SpawnRotations[ChosenIndex];
			}
			else
			{
				SetRotation = false;
			}

			if (SetRotation == true)
			{
				PossessPawn(GetControlledPawn()->GetClass(), nullptr, true, Location, Rotation, false, false);
			}
			else
			{
				PossessPawn(GetControlledPawn()->GetClass(), nullptr, true, Location, FRotator::ZeroRotator, false, true);
			}
		}
	}

	if (ShowHUDOnRespawn == true)
	{
		CreateUIWidget(HUDClass, 0, RemoveAllWidgetsOnRespawn, DeathScreen, true, false, true, true, false, false);
	}
	else
	{
		RemoveUIWidget(DeathScreen, RemoveAllWidgetsOnRespawn, true, false, true, true);
	}
}

void AMultiplayerPlayerController::ServerRespawn1_Implementation()
{
	Respawn1();
}

void AMultiplayerPlayerController::SetPlayerIndex(int NewPlayerIndex)
{
	PlayerIndex = NewPlayerIndex;
}

int AMultiplayerPlayerController::GetPlayerIndex()
{
	return PlayerIndex;
}

void AMultiplayerPlayerController::SetCanSwitchPerspective(bool NewCanSwitchPerspective)
{
	CanSwitchPerspective = NewCanSwitchPerspective;
}

bool AMultiplayerPlayerController::GetCanSwitchPerspective()
{
	return CanSwitchPerspective;
}

void AMultiplayerPlayerController::SetUsingThirdPerson(bool NewUsingThirdPerson)
{
	UsingThirdPerson = NewUsingThirdPerson;
}

bool AMultiplayerPlayerController::GetUsingThirdPerson()
{
	return UsingThirdPerson;
}

void AMultiplayerPlayerController::SetGiveLoadoutOnBeginPlay(bool NewGiveLoadoutOnBeginPlay)
{
	GiveLoadoutOnBeginPlay = NewGiveLoadoutOnBeginPlay;
}

bool AMultiplayerPlayerController::GetGiveLoadoutOnBeginPlay()
{
	return GiveLoadoutOnBeginPlay;
}

void AMultiplayerPlayerController::SetMaxWeaponAmount(int NewMaxWeaponAmount, bool DestroyWeaponsInExcess)
{
	MaxWeaponAmount = NewMaxWeaponAmount;

	if (GetControlledPawn())
	{
		if (AMultiplayerCharacter* PlayerCast = Cast<AMultiplayerCharacter>(GetControlledPawn()))
		{
			if (PlayerCast->GetAmountOfWeapons() > NewMaxWeaponAmount)
			{
				PlayerCast->RemoveWeaponPastIndex(NewMaxWeaponAmount, DestroyWeaponsInExcess);
			}
		}
	}
}

int AMultiplayerPlayerController::GetMaxWeaponAmount()
{
	return MaxWeaponAmount;
}

void AMultiplayerPlayerController::SetRandomizeUnselectedWeapons(bool NewRandomizeUnselectedWeapons)
{
	RandomizeUnselectedWeapons = NewRandomizeUnselectedWeapons;
}

bool AMultiplayerPlayerController::GetRandomizeUnselectedWeapons()
{
	return RandomizeUnselectedWeapons;
}

void AMultiplayerPlayerController::SetRespawnPoints(TMap<FVector, FRotator> NewRespawnPoints)
{
	RespawnPoints = NewRespawnPoints;
}

TMap<FVector, FRotator> AMultiplayerPlayerController::GetRespawnPoints()
{
	return RespawnPoints;
}

void AMultiplayerPlayerController::SetAvoidDuplicatesForRandomWeapons(int NewAvoidDuplicatesForRandomWeapons)
{
	AvoidDuplicatesForRandomWeapons = NewAvoidDuplicatesForRandomWeapons;
}

int AMultiplayerPlayerController::GetAvoidDuplicatesForRandomWeapons()
{
	return AvoidDuplicatesForRandomWeapons;
}

void AMultiplayerPlayerController::SetRespawnDelay(float NewRespawnDelay)
{
	RespawnDelay = NewRespawnDelay;
}

float AMultiplayerPlayerController::GetRespawnDelay()
{
	return RespawnDelay;
}

void AMultiplayerPlayerController::SetAllSharedCalibersOnSpawn(TMap<FName, int32> NewAllSharedCalibersOnSpawn)
{
	AllSharedCalibersOnSpawn = NewAllSharedCalibersOnSpawn;
}

TMap<FName, int32> AMultiplayerPlayerController::GetAllSharedCalibersOnSpawn()
{
	return AllSharedCalibersOnSpawn;
}

void AMultiplayerPlayerController::SetControllerVibration(bool NewControllerVibration)
{
	ControllerVibration = NewControllerVibration;
	bForceFeedbackEnabled = NewControllerVibration;
}

void AMultiplayerPlayerController::VibrateController(UForceFeedbackEffect* ForceFeedbackEffect, FName Tag, bool Looping, bool IgnoreTimeDilation, bool PlayWhilePaused)
{
	if (ControllerVibration == true && UsingGamepad == true && ForceFeedbackEffect)
	{
		FForceFeedbackParameters ForceFeedbackParameters;
		ForceFeedbackParameters.Tag = Tag;
		ForceFeedbackParameters.bLooping = Looping;
		ForceFeedbackParameters.bIgnoreTimeDilation = IgnoreTimeDilation;
		ForceFeedbackParameters.bPlayWhilePaused = PlayWhilePaused;

		ClientPlayForceFeedback(ForceFeedbackEffect, ForceFeedbackParameters);
	}
}

void AMultiplayerPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerPlayerController, PlayerIndex);
	DOREPLIFETIME(AMultiplayerPlayerController, WeaponChoices);
	DOREPLIFETIME(AMultiplayerPlayerController, MaxWeaponAmount);
	DOREPLIFETIME(AMultiplayerPlayerController, UsingThirdPerson);
}