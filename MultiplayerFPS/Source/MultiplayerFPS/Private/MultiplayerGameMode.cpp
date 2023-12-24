// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerGameMode.h"
#include "MultiplayerPlayerController.h"

void AMultiplayerGameMode::PostLogin(APlayerController* NewPlayer)
{
	AddPlayerController(NewPlayer);
}

void AMultiplayerGameMode::Logout(AController* ExitingPlayer)
{
	if (APlayerController* ControllerCast = Cast<APlayerController>(ExitingPlayer))
	{
		RemovePlayerController(ControllerCast);
	}
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
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NewController Cast Failed MultiplayerGameMode.cpp:AddPlayerController");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NewController Invalid MultiplayerGameMode.cpp:AddPlayerController");
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
		else if (!ControllerToRemove)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "ControllerToRemove Invalid MultiplayerGameMode.cpp:RemovePlayerController");
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "ControllerToRemove Not In AllPlayerControllers Array MultiplayerGameMode.cpp:RemovePlayerController");
		}
	}
	else if (Index <= AllPlayerControllers.Num() - 1)
	{
		AllPlayerControllers.RemoveAt(Index);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Invalid Array Index MultiplayerGameMode.cpp:RemovePlayerController");
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
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NewPawn Invalid MultiplayerGameMode.cpp:AddPlayerPawn");
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
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Invalid Array Index MultiplayerGameMode.cpp:RemovePlayerPawn");
	}
}

TArray<APawn*> AMultiplayerGameMode::GetAllPlayerPawns()
{
	return AllPlayerPawns;
}