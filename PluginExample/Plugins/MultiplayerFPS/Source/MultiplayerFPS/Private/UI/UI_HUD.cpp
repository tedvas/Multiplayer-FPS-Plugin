// Fill out your copyright notice in the Description page of Project Settings.
// the reason you'll see weird parenthesis on if statements is for linux compatibility

#include "UI/UI_HUD.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerHealthComponent.h"

float UUI_HUD::GetHealth()
{
	if (UIGetMultiplayerCharacter())
	{
		if (GetHealthComponent())
		{
			return UKismetMathLibrary::Conv_IntToDouble(GetHealthComponent()->GetHealth()) / UKismetMathLibrary::Conv_IntToDouble(GetHealthComponent()->GetMaxHealth());
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		return 0.0f;
	}
}

UMultiplayerHealthComponent* UUI_HUD::GetHealthComponent()
{
	if (!HealthComponent)
	{
		if (UIGetMultiplayerCharacter())
		{
			HealthComponent = UIGetMultiplayerCharacter()->GetHealthComponent();
		}
		else
		{
			return nullptr;
		}
	}

	return HealthComponent;
}

FText UUI_HUD::GetHealthNumber()
{
	if (UIGetMultiplayerCharacter())
	{
		if (GetHealthComponent())
		{
			return FText::FromString(FString::FromInt(GetHealthComponent()->GetHealth()));
		}
		else
		{
			return FText::FromString("0");
		}
	}
	else
	{
		return FText::FromString("0");
	}
}

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

int UUI_HUD::GetAmmo()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			return Weapon->GetAmmoInMagazine();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

int UUI_HUD::GetReserveAmmo()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetUseSharedCalibers() == true)
			{
				return UIGetMultiplayerCharacter()->GetSharedCaliberAmount(Weapon->GetCaliberToUse());
			}
			else
			{
				return Weapon->GetReserveAmmo();
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

ESlateVisibility UUI_HUD::VisibleIfUsesSharedCalibers()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetUseSharedCalibers() == true && Weapon->GetDoesOverheat() == false)
			{
				return ESlateVisibility::SelfHitTestInvisible;
			}
			else
			{
				return ESlateVisibility::Collapsed;
			}
		}
		else
		{
			return ESlateVisibility::Collapsed;
		}
	}
	else
	{
		return ESlateVisibility::Collapsed;
	}
}

FText UUI_HUD::GetReserveAmmoCaliberName()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetUseSharedCalibers() == true)
			{
				return FText().FromName(UIGetMultiplayerCharacter()->GetSharedCaliberName(Weapon->GetCaliberToUse()));
			}
			else
			{
				return FText();
			}
		}
		else
		{
			return FText();
		}
	}
	else
	{
		return FText();
	}
}

float UUI_HUD::GetWeaponOverheat()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			return Weapon->GetCurrentHeat() / Weapon->GetMaxHeat();
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		return 0.0f;
	}
}

ESlateVisibility UUI_HUD::AmmoVisibility()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if ((Weapon->GetDoesOverheat() == true && Weapon->GetInfiniteAmmo() != 1) || Weapon->GetInfiniteAmmo() == 2)
			{
				return ESlateVisibility::Collapsed;
			}
			else
			{
				return ESlateVisibility::Visible;
			}
		}
		else
		{
			return ESlateVisibility::Visible;
		}
	}
	else
	{
		return ESlateVisibility::Visible;
	}
}

ESlateVisibility UUI_HUD::ReserveAmmoVisibility()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetDoesOverheat() == true || Weapon->GetInfiniteAmmo() >= 1)
			{
				return ESlateVisibility::Collapsed;
			}
			else
			{
				return ESlateVisibility::Visible;
			}
		}
		else
		{
			return ESlateVisibility::Visible;
		}
	}
	else
	{
		return ESlateVisibility::Visible;
	}
}

ESlateVisibility UUI_HUD::OverheatVisibility()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetDoesOverheat() == true && Weapon->GetInfiniteAmmo() == 0)
			{
				return ESlateVisibility::Visible;
			}
			else
			{
				return ESlateVisibility::Collapsed;
			}
		}
		else
		{
			return ESlateVisibility::Collapsed;
		}
	}
	else
	{
		return ESlateVisibility::Collapsed;
	}
}

ESlateVisibility UUI_HUD::InfiniteAmmoVisibility()
{
	if (UIGetMultiplayerCharacter())
	{
		if (AMultiplayerGun* Weapon = UIGetMultiplayerCharacter()->GetWeapon(true))
		{
			if (Weapon->GetInfiniteAmmo() == 0)
			{
				return ESlateVisibility::Collapsed;
			}
			else
			{
				return ESlateVisibility::Visible;
			}
		}
		else
		{
			return ESlateVisibility::Visible;
		}
	}
	else
	{
		return ESlateVisibility::Visible;
	}
}

ESlateVisibility UUI_HUD::CrosshairVisibility()
{
	ESlateVisibility NewVisibility = ESlateVisibility::Collapsed;

	if (UIGetMultiplayerCharacter())
	{
		if (UIGetMultiplayerCharacter()->GetAmountOfWeapons() >= 1 && (UIGetMultiplayerCharacter()->GetIsADSing() == false || UIGetMultiplayerCharacter()->GetUsingThirdPerson() == true))
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
