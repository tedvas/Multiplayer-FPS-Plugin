// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerGun.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerPlayerController.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMultiplayerPlayerController();

	UFUNCTION(BlueprintCallable, meta = (Tooltip = "This is exactly the same as the engine function, the reason it is BlueprintCallable is because you can override this function"), Category = "Functions")
	virtual APawn* GetControlledPawn();

	UFUNCTION(BlueprintCallable, Client, Reliable, meta = (Tooltip = "Only pause offline, does not work in multiplayer"), Category = "Functions")
	virtual void CreateUIWidget(TSubclassOf<UUserWidget> NewWidget, int32 ZOrder = 0, bool RemoveAllWidgetsFirst = false, UUserWidget* WidgetToRemove = nullptr, bool ChangeCursor = false, bool ShowCursor = false, bool ChangeInput = false, bool GameInput = true, bool ChangePaused = false, bool PauseGame = false);

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void RemoveUIWidget(UUserWidget* WidgetToRemove, bool RemoveAllWidgets = false, bool ChangeCursor = false, bool ShowCursor = false, bool ChangeInput = false, bool GameInput = true, bool ChangePaused = false, bool PauseGame = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	virtual UUserWidget* GetUILastIndex();

	UFUNCTION(BlueprintCallable, meta = (Tooltip = "Velocity options only apply to characters"), Category = "Functions")
	virtual void PossessPawn(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation = true, bool KeepVelocity = false, bool ChangeStartingVelocity = false, FVector NewVelocity = FVector::ZeroVector);

	UFUNCTION(Server, Reliable, meta = (Tooltip = "Velocity options only apply to characters"), Category = "Functions")
	virtual void ServerPossessPawn(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation = true, bool KeepVelocity = false, bool ChangeStartingVelocity = false, FVector NewVelocity = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void ApplySettingsToCharacter();

	UFUNCTION(Category = "Functions")
	virtual void PrintStringForControlledPawnInvalidApplySettings();

	UFUNCTION(BlueprintCallable, Client, Reliable, Category = "Functions")
	virtual void ClientSetControlRotation(FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Respawn(float DelayToRespawn = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void Respawn1();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void ServerRespawn1();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void GiveLoadout();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetPlayerIndex(int NewPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual int GetPlayerIndex();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetGiveLoadoutOnBeginPlay(bool NewGiveLoadoutOnBeginPlay);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetGiveLoadoutOnBeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetMaxWeaponAmount(int NewMaxWeaponAmount, bool DestroyWeaponsInExcess = false);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual int GetMaxWeaponAmount();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetRandomizeUnselectedWeapons(bool NewRandomizeUnselectedWeapons);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual bool GetRandomizeUnselectedWeapons();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetRespawnPoints(TMap<FVector, FRotator> NewRespawnPoints);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual TMap<FVector, FRotator> GetRespawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAvoidDuplicatesForRandomWeapons(int NewAvoidDuplicatesForRandomWeapons);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual int GetAvoidDuplicatesForRandomWeapons();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetRespawnDelay(float NewRespawnDelay);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual float GetRespawnDelay();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetAllSharedCalibersOnSpawn(TMap<FName, int32> NewAllSharedCalibersOnSpawn);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual TMap<FName, int32> GetAllSharedCalibersOnSpawn();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetControllerVibration(bool NewControllerVibration);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void VibrateController(UForceFeedbackEffect* ForceFeedbackEffect, FName Tag = "", bool Looping = false, bool IgnoreTimeDilation = false, bool PlayWhilePaused = false);

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TArray<UUserWidget*> AllUI;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* HUD;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* DeathScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (Tooltip = "This is used to have a reference to the HUD separate from the rest of the UI"))
	TSubclassOf<UUserWidget> HUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (Tooltip = "This is used to have a reference to the DeathScreen separate from the rest of the UI"))
	TSubclassOf<UUserWidget> DeathScreenClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (Tooltip = "This will be used when spawning the player to give them their loadout"))
	TArray<TSubclassOf<AMultiplayerGun>> WeaponChoices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This is all the weapons in the game, make sure to add all the playable weapons to this array variable"))
	TArray<TSubclassOf<AMultiplayerGun>> AllWeaponClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This is all the weapons in the game, make sure to add all the playable weapons to this array variable, this is only for a loadout menu"))
	TMap<FName, TSubclassOf<AMultiplayerGun>> AllWeaponClassesForUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = 0.0f))
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keyboard And Mouse", meta = (Tooltip = "Mouse sensitivity on the X axis when not aiming"))
	float MouseDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keyboard And Mouse", meta = (Tooltip = "Mouse sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float MouseAimingSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keyboard And Mouse", meta = (Tooltip = "Mouse sensitivity on the Y axis when not aiming"))
	float MouseDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keyboard And Mouse", meta = (Tooltip = "Mouse sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float MouseAimingSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Keyboard And Mouse")
	float MouseAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad", meta = (Tooltip = "Gamepad sensitivity on the X axis when not aiming"))
	float GamepadDefaultSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad", meta = (Tooltip = "Gamepad sensitivity on the X axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float GamepadAimingSensitivityX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad", meta = (Tooltip = "Gamepad sensitivity on the Y axis when not aiming"))
	float GamepadDefaultSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad", meta = (Tooltip = "Gamepad sensitivity on the Y axis when aiming, only applies if UseAimSensitivityMultipler is false"))
	float GamepadAimingSensitivityY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad")
	float GamepadAimingSensitivityMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gamepad", meta = (Tooltip = "Do not set this variable directly, instead use the SetControllerVibration function"))
	bool ControllerVibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool UseAimSensitivityMultipler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool ToggleAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool HoldButtonToJump;

protected:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
	int PlayerIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Variables", meta = (Tooltip = "This will just detect whether the last input was with a keyboard and mouse or a controller"))
	bool UsingGamepad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "Only applies to MultiplayerCharacter when possessing pawn"))
	bool GiveLoadoutOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (ClampMin = 0))
	int MaxWeaponAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool RandomizeUnselectedWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "0 = will allow duplicates, 1 = will avoid duplicates unless there aren't enough weapons to choose from, 2 = will avoid duplicates no matter what", ClampMin = 0, ClampMax = 2))
	int AvoidDuplicatesForRandomWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This would be the name and amount of each caliber the player spawns with, only applies to guns that use this rather than their own reserve ammo", ClampMin = 0))
	TMap<FName, int32> AllSharedCalibersOnSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (ClampMin = 0.0f))
	float RespawnDelay;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
	TMap<FVector, FRotator> RespawnPoints;

	FTimerHandle GetControlledPawnTimerHandle;
	FTimerHandle RespawnTimerHandle;
	FTimerDelegate ShowHUDTimerDelegate;
	FTimerHandle ShowHUDTimerHandle;
};