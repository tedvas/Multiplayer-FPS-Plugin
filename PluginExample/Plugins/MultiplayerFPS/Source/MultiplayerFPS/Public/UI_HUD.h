// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetMasterClass.h"
#include "UI_HUD.generated.h"

UCLASS()
class MULTIPLAYERFPS_API UUI_HUD : public UWidgetMasterClass
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	float GetHealth();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	UMultiplayerHealthComponent* GetHealthComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	FText GetHealthNumber();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	FText GetWeaponName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility VisibleIfHasWeapon();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	int GetAmmo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	int GetReserveAmmo();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility VisibleIfUsesSharedCalibers();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	FText GetReserveAmmoCaliberName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	float GetWeaponOverheat();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility AmmoVisibility();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility ReserveAmmoVisibility();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility OverheatVisibility();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility InfiniteAmmoVisibility();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Functions")
	ESlateVisibility CrosshairVisibility();

protected:

	UPROPERTY()
	UMultiplayerHealthComponent* HealthComponent;
};