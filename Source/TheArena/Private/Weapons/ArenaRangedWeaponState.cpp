// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeaponState.h"

UArenaRangedWeaponState::UArenaRangedWeaponState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{	
	SetIsReplicated(true);
	bReplicates = true;

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

bool UArenaRangedWeaponState::GetCoverTargeting()
{
	return CoverTargeting;
}
void UArenaRangedWeaponState::SetCoverTargeting(bool Value)
{
	CoverTargeting = Value;
}

void UArenaRangedWeaponState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UArenaRangedWeaponState, WeaponState);
	DOREPLIFETIME(UArenaRangedWeaponState, TargetingState);
	DOREPLIFETIME(UArenaRangedWeaponState, EquippedState);
	DOREPLIFETIME(UArenaRangedWeaponState, CoverTargeting);
}
