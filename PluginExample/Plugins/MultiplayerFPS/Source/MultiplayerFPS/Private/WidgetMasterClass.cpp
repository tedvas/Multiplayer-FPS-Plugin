// Fill out your copyright notice in the Description page of Project Settings.

#include "WidgetMasterClass.h"

AMultiplayerCharacter* UWidgetMasterClass::UIGetMultiplayerCharacter()
{
	if (!MultiplayerCharacter)
	{
		MultiplayerCharacter = Cast<AMultiplayerCharacter>(GetOwningPlayerPawn());
	}

	return MultiplayerCharacter;
}

AMultiplayerPlayerController* UWidgetMasterClass::UIGetMultiplayerController()
{
	if (!MultiplayerController)
	{
		MultiplayerController = Cast<AMultiplayerPlayerController>(GetOwningPlayer());
	}

	return MultiplayerController;
}