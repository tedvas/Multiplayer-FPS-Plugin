// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerCharacter.h"
#include "MultiplayerGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"

AMultiplayerPlayerController::AMultiplayerPlayerController()
{
	PlayerIndex = 0;
	GiveLoadoutOnBeginPlay = true;
	MaxWeaponAmount = 2;
	RandomizeUnselectedWeapons = true;

	bReplicates = true;
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
			CreatedWidget->AddToViewport(ZOrder);
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
				UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, GetUILastIndex());
			}
		}

		if (ChangePaused == true)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), PauseGame);
		}
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
			UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, GetUILastIndex());
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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "GetUILastIndex returned nullptr, MultiplayerPlayerController.cpp");
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

	if (SpawnNewPawn == true && NewPawnToSpawn || SpawnNewPawn == false && NewPawn)
	{
		FRotator CurrentControlRotation = GetControlRotation();
		FVector CurrentVeloctiy = FVector(0.0f, 0.0f, 0.0f);

		if (KeepVelocity == true)
		{
			if (ACharacter* CurrentCharacterCast = Cast<ACharacter>(ControlledPawn))
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
			ControlledPawn = NewPawnToPossess;
			Possess(NewPawnToPossess);

			if (KeepControlRotation == true)
			{
				ClientSetControlRotation(CurrentControlRotation);
			}
			else
			{
				ClientSetControlRotation(Rotation);
			}

			if (KeepVelocity == true || ChangeStartingVelocity == true)
			{
				if (AMultiplayerCharacter* NewPlayerCharacterCast = Cast<AMultiplayerCharacter>(NewPawnToPossess))
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

void AMultiplayerPlayerController::ClientSetControlRotation_Implementation(FRotator Rotation)
{
	SetControlRotation(Rotation);
}

void AMultiplayerPlayerController::GiveLoadout_Implementation()
{
	if (ControlledPawn)
	{
		if (AMultiplayerCharacter* NewCharacterCast = Cast<AMultiplayerCharacter>(ControlledPawn))
		{
			NewCharacterCast->GiveLoadout(WeaponChoices, MaxWeaponAmount);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Character Invalid MultiplayerPlayerController.cpp:GiveLoadout");
	}
}

void AMultiplayerPlayerController::Respawn(float DelayToRespawn)
{
	if (DelayToRespawn > 0)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMultiplayerPlayerController::Respawn1, DelayToRespawn, false, DelayToRespawn);

		ShowHUDTimerDelegate.BindUFunction(this, FName("CreateUIWidget"), HUDClass, 0, false, DeathScreen, true, false, true, true, false, false);
		GetWorldTimerManager().SetTimer(ShowHUDTimerHandle, ShowHUDTimerDelegate, DelayToRespawn, false, DelayToRespawn);
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

	if (ControlledPawn)
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
				PossessPawn(ControlledPawn->GetClass(), nullptr, true, Location, Rotation, false, false);
			}
			else
			{
				PossessPawn(ControlledPawn->GetClass(), nullptr, true, Location, FRotator::ZeroRotator, false, true);
			}
		}
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

void AMultiplayerPlayerController::SetGiveLoadoutOnBeginPlay(bool NewGiveLoadoutOnBeginPlay)
{
	GiveLoadoutOnBeginPlay = NewGiveLoadoutOnBeginPlay;
}

bool AMultiplayerPlayerController::GetGiveLoadoutOnBeginPlay()
{
	return GiveLoadoutOnBeginPlay;
}

void AMultiplayerPlayerController::SetMaxWeaponAmount(int NewMaxWeaponAmount)
{
	MaxWeaponAmount = NewMaxWeaponAmount;
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

void AMultiplayerPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerPlayerController, PlayerIndex);
	DOREPLIFETIME(AMultiplayerPlayerController, MaxWeaponAmount);
	DOREPLIFETIME(AMultiplayerPlayerController, ControlledPawn);
}