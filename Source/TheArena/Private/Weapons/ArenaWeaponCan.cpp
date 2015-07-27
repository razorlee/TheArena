// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaWeaponCan.h"

bool ArenaWeaponCan::Equip(AArenaCharacter* character, AArenaWeapon* weapon)
{
	return true;
}

bool ArenaWeaponCan::UnEquip(AArenaCharacter* character, AArenaWeapon* weapon)
{
	return true;
}

bool ArenaWeaponCan::Activate(AArenaCharacter* character, AArenaWeapon* weapon)
{
	return true;
}

bool ArenaWeaponCan::Fire(AArenaCharacter* character, AArenaWeapon* weapon)
{
	if (character
		&& weapon->GetWeaponAttributes()->CurrentClip > 0
		&& (weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Idle
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Default
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Firing))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaWeaponCan::Reload(AArenaCharacter* character, AArenaWeapon* weapon)
{
	if (weapon->GetWeaponAttributes()->CurrentClip < weapon->GetWeaponAttributes()->GetCapacity()
		&& weapon->GetWeaponAttributes()->TotalAmmo - weapon->GetWeaponAttributes()->CurrentClip > 0
		&& (weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Idle
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Default
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Firing))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaWeaponCan::Melee(AArenaCharacter* character, AArenaWeapon* weapon)
{
	if (character
		&& (weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Idle
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Default
		|| weapon->GetWeaponState()->GetWeaponState() == EWeaponState::Firing))
	{
		return true;
	}
	else
	{
		return false;
	}
}