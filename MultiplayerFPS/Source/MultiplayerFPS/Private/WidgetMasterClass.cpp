// Fill out your copyright notice in the Description page of Project Settings.

#include "WidgetMasterClass.h"

AMultiplayerCharacter* UWidgetMasterClass::UIGetMultiplayerCharacter()
{
	return Cast<AMultiplayerCharacter>(GetOwningPlayerPawn());
}

AMultiplayerPlayerController* UWidgetMasterClass::UIGetMultiplayerController()
{
	return Cast<AMultiplayerPlayerController>(GetOwningPlayer());
}