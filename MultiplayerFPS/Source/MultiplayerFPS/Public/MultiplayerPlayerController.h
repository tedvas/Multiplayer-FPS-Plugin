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

	UFUNCTION(BlueprintCallable, Client, Reliable, meta = (Tooltip = "Only pause offline, does not work in multiplayer"))
	virtual void CreateUIWidget(TSubclassOf<UUserWidget> NewWidget, int32 ZOrder = 0, bool RemoveAllWidgetsFirst = false, UUserWidget* WidgetToRemove = nullptr, bool ChangeCursor = false, bool ShowCursor = false, bool ChangeInput = false, bool GameInput = true, bool ChangePaused = false, bool PauseGame = false);

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void RemoveUIWidget(UUserWidget* WidgetToRemove, bool RemoveAllWidgets = false, bool ChangeCursor = false, bool ShowCursor = false, bool ChangeInput = false, bool GameInput = true, bool ChangePaused = false, bool PauseGame = false);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual UUserWidget* GetUILastIndex();

	UFUNCTION(BlueprintCallable, meta = (Tooltip = "Velocity options only apply to characters"))
	virtual void PossessPawn(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation = true, bool KeepVelocity = false, bool ChangeStartingVelocity = false, FVector NewVelocity = FVector::ZeroVector);

	UFUNCTION(Server, Reliable, meta = (Tooltip = "Velocity options only apply to characters"))
	virtual void ServerPossessPawn(TSubclassOf<APawn> NewPawnToSpawn, APawn* NewPawn, bool SpawnNewPawn, FVector Location, FRotator Rotation, bool DestroyOldPawn, bool KeepControlRotation = true, bool KeepVelocity = false, bool ChangeStartingVelocity = false, FVector NewVelocity = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Client, Reliable)
	virtual void ClientSetControlRotation(FRotator Rotation);

	UFUNCTION(BlueprintCallable)
	virtual void Respawn(float DelayToRespawn = 0.0f);

	UFUNCTION(BlueprintCallable)
	virtual void Respawn1();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void ServerRespawn1();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void GiveLoadout();

	UFUNCTION(BlueprintCallable)
	virtual void SetPlayerIndex(int NewPlayerIndex);

	UFUNCTION(BlueprintCallable)
	virtual int GetPlayerIndex();

	UFUNCTION(BlueprintCallable)
	virtual void SetGiveLoadoutOnBeginPlay(bool NewGiveLoadoutOnBeginPlay);

	UFUNCTION(BlueprintCallable)
	virtual bool GetGiveLoadoutOnBeginPlay();

	UFUNCTION(BlueprintCallable)
	virtual void SetMaxWeaponAmount(int NewMaxWeaponAmount);

	UFUNCTION(BlueprintCallable)
	virtual int GetMaxWeaponAmount();

	UFUNCTION(BlueprintCallable)
	virtual void SetRandomizeUnselectedWeapons(bool NewRandomizeUnselectedWeapons);

	UFUNCTION(BlueprintCallable)
	virtual bool GetRandomizeUnselectedWeapons();

	UFUNCTION(BlueprintCallable)
	virtual void SetRespawnPoints(TMap<FVector, FRotator> NewRespawnPoints);

	UFUNCTION(BlueprintCallable)
	virtual TMap<FVector, FRotator> GetRespawnPoints();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<UUserWidget*> AllUI;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* HUD;

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* DeathScreen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (Tooltip = "This is used to have a reference to the HUD separate from the rest of the UI"))
	TSubclassOf<UUserWidget> HUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (Tooltip = "This is used to have a reference to the DeathScreen separate from the rest of the UI"))
	TSubclassOf<UUserWidget> DeathScreenClass;

	UPROPERTY(BlueprintReadWrite, Replicated)
	APawn* ControlledPawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "This will be used when spawning the player to give them their loadout"))
	TArray<TSubclassOf<AMultiplayerGun>> WeaponChoices;

protected:

	UPROPERTY(BlueprintReadWrite, Replicated)
	int PlayerIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons", meta = (Tooltip = "Only applies to MultiplayerCharacter when possessing pawn"))
	bool GiveLoadoutOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Weapons", meta = (ClampMin = 0))
	int MaxWeaponAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	bool RandomizeUnselectedWeapons;

	UPROPERTY(BlueprintReadWrite)
	TMap<FVector, FRotator> RespawnPoints;

	FTimerHandle RespawnTimerHandle;
	FTimerDelegate ShowHUDTimerDelegate;
	FTimerHandle ShowHUDTimerHandle;
};