// Fill out your copyright notice in the Description page of Project Settings.

#include "UI_HUD.h"
#include "Kismet/KismetMathLibrary.h"

FText UUI_HUD::GetWeaponName()
{
	FText WeaponsName = FText::FromString("");

	if (UIGetMultiplayerCharacter())
	{
		if (UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			WeaponsName = FText::FromName(UIGetMultiplayerCharacter()->GetWeapon(true)->WeaponName);
		}
	}

	return WeaponsName;
}

ESlateVisibility UUI_HUD::VisibleIfHasWeapon()
{
	ESlateVisibility NewVisibility = ESlateVisibility::Collapsed;

	if (UIGetMultiplayerCharacter())
	{
		if (UIGetMultiplayerCharacter()->GetAmountOfWeapons() >= 1)
		{
			NewVisibility = ESlateVisibility::Visible;
		}
		else
		{
			NewVisibility = ESlateVisibility::Collapsed;
		}
	}

	return NewVisibility;
}

ESlateVisibility UUI_HUD::CrosshairVisibility()
{
	ESlateVisibility NewVisibility = ESlateVisibility::Collapsed;

	if (UIGetMultiplayerCharacter())
	{
		if (UIGetMultiplayerCharacter()->GetAmountOfWeapons() >= 1 && UIGetMultiplayerCharacter()->GetIsADSing() == false)
		{
			NewVisibility = ESlateVisibility::Visible;
		}
		else
		{
			NewVisibility = ESlateVisibility::Collapsed;
		}
	}

	return NewVisibility;
}