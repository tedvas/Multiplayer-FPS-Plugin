// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetMasterClass.h"
#include "Components/Progressbar.h"
#include "UI_HUD.generated.h"

UCLASS()
class MULTIPLAYERFPS_API UUI_HUD : public UWidgetMasterClass
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FText GetWeaponName();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ESlateVisibility VisibleIfHasWeapon();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ESlateVisibility CrosshairVisibility();
};