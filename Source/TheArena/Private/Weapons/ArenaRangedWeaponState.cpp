// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeaponState.h"

UArenaRangedWeaponState::UArenaRangedWeaponState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{	
	WeaponState = EWeaponState::Default;
	TargetingState = ETargetingState::Default;
	EquippedState = EEquippedState::UnEquipped;
}

EWeaponState::Type UArenaRangedWeaponState::GetWeaponState()
{
	return WeaponState;
}
void UArenaRangedWeaponState::SetWeaponState(EWeaponState::Type NewState)
{
	WeaponState = NewState;
}

ETargetingState::Type UArenaRangedWeaponState::GetTargetingState()
{
	return TargetingState;
}
void UArenaRangedWeaponState::SetTargetingState(ETargetingState::Type NewState)
{
	TargetingState = NewState;
}

EEquippedState::Type UArenaRangedWeaponState::GetEquippedState()
{
	return EquippedState;
}
void UArenaRangedWeaponState::SetEquippedState(EEquippedState::Type NewState)
{
	EquippedState = NewState;
}


