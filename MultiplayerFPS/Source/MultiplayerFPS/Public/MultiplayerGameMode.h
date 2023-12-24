// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerGameMode.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitingPlayer) override;

	UFUNCTION(BlueprintCallable)
	virtual void AddPlayerController(APlayerController* NewController);

	UFUNCTION(BlueprintCallable)
	virtual void RemovePlayerController(APlayerController* ControllerToRemove, bool RemoveByIndex = false, int Index = 0);

	UFUNCTION(BlueprintCallable)
	virtual TArray<APlayerController*> GetAllPlayerControllers();

	UFUNCTION(BlueprintCallable)
	virtual void AddPlayerPawn(APawn* NewPawn);

	UFUNCTION(BlueprintCallable)
	virtual void RemovePlayerPawn(APawn* PawnToRemove, bool RemoveByIndex = false, int Index = 0);

	UFUNCTION(BlueprintCallable)
	virtual TArray<APawn*> GetAllPlayerPawns();

protected:

	UPROPERTY(BlueprintReadWrite)
	TArray<APlayerController*> AllPlayerControllers;

	UPROPERTY(BlueprintReadWrite)
	TArray<APawn*> AllPlayerPawns;
};