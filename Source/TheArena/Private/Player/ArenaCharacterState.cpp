// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterState.h"

UArenaCharacterState::UArenaCharacterState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerState = EPlayerState::Default;
	CoverState = ECoverState::Default;
	CombatState = ECombatState::Passive;
}

void UArenaCharacterState::Reset()
{
	PlayerState = EPlayerState::Default;
	CoverState = ECoverState::Default;
	CombatState = ECombatState::Passive;
}

EPlayerState::Type UArenaCharacterState::GetPlayerState() const
{
	return PlayerState;
}

void UArenaCharacterState::SetPlayerState(EPlayerState::Type State)
{
	PlayerState = State;
}

ECoverState::Type UArenaCharacterState::GetCoverState() const
{
	return CoverState;
}

void UArenaCharacterState::SetCoverState(ECoverState::Type State)
{
	CoverState = State;
}

ECombatState::Type UArenaCharacterState::GetCombatState() const
{
	return CombatState;
}

void UArenaCharacterState::SetCombatState(ECombatState::Type State)
{
	CombatState = State;
}

