// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerGameMode.h"
#include "MultiplayerPlayerController.h"

void AMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	AddPlayerController(NewPlayer);
	DispatchPostLogin(NewPlayer);
}

void AMultiplayerGameMode::Logout(AController* ExitingPlayer)
{
	if (APlayerController* ControllerCast = Cast<APlayerController>(ExitingPlayer))
	{
		RemovePlayerController(ControllerCast);
	}
	
	Super::Logout(ExitingPlayer);
}

void AMultiplayerGameMode::AddPlayerController(APlayerController* NewController)
{
	if (NewController)
	{
		AllPlayerControllers.AddUnique(NewController);

		if (AMultiplayerPlayerController* ControllerCast = Cast<AMultiplayerPlayerController>(NewController))
		{
			ControllerCast->SetPlayerIndex(AllPlayerControllers.Find(NewController));
		}
	}
}

void AMultiplayerGameMode::RemovePlayerController(APlayerController* ControllerToRemove, bool RemoveByIndex, int Index)
{
	if (RemoveByIndex == false)
	{
		if (ControllerToRemove && AllPlayerControllers.Contains(ControllerToRemove))
		{
			AllPlayerControllers.Remove(ControllerToRemove);
		}
	}
	else if (Index <= AllPlayerControllers.Num() - 1)
	{
		AllPlayerControllers.RemoveAt(Index);
	}
}

TArray<APlayerController*> AMultiplayerGameMode::GetAllPlayerControllers()
{
	return AllPlayerControllers;
}

void AMultiplayerGameMode::AddPlayerPawn(APawn* NewPawn)
{
	if (NewPawn)
	{
		AllPlayerPawns.Add(NewPawn);
	}
}

void AMultiplayerGameMode::RemovePlayerPawn(APawn* PawnToRemove, bool RemoveByIndex, int Index)
{
	if (RemoveByIndex == false)
	{
		if (PawnToRemove && AllPlayerPawns.Contains(PawnToRemove))
		{
			AllPlayerPawns.Remove(PawnToRemove);
		}
	}
	else if (Index <= AllPlayerPawns.Num() - 1)
	{
		AllPlayerPawns.RemoveAt(Index);
	}
}

TArray<APawn*> AMultiplayerGameMode::GetAllPlayerPawns()
{
	return AllPlayerPawns;
}