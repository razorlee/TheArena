// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaPlayerState.h"


AArenaPlayerState::AArenaPlayerState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerState = EPlayerState::Default;
	CoverState = ECoverState::Default;
	CombatState = ECombatState::Passive;
}

void AArenaPlayerState::Reset()
{
	Super::Reset();
}

void AArenaPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);
}

EPlayerState::Type AArenaPlayerState::GetPlayerState() const
{
	return PlayerState;
}

void AArenaPlayerState::SetPlayerState(EPlayerState::Type State)
{
	PlayerState = State;
}

ECoverState::Type AArenaPlayerState::GetCoverState() const
{
	return CoverState;
}

void AArenaPlayerState::SetCoverState(ECoverState::Type State)
{
	CoverState = State;
}

ECombatState::Type AArenaPlayerState::GetCombatState() const
{
	return CombatState;
}

void AArenaPlayerState::SetCombatState(ECombatState::Type State)
{
	CombatState = State;
}


