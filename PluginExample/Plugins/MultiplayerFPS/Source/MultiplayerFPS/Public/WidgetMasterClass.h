// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerCharacter.h"
#include "MultiplayerPlayerController.h"
#include "WidgetMasterClass.generated.h"

UCLASS()
class MULTIPLAYERFPS_API UWidgetMasterClass : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	AMultiplayerCharacter* UIGetMultiplayerCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	AMultiplayerPlayerController* UIGetMultiplayerController();

protected:

	UPROPERTY()
	AMultiplayerCharacter* MultiplayerCharacter;

	UPROPERTY()
	AMultiplayerPlayerController* MultiplayerController;
};