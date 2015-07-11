// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "Net/UnrealNetwork.h"
#include "ArenaCharacterState.h"

UArenaCharacterState::UArenaCharacterState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SetIsReplicated(true);
	bReplicates = true;

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

class AArenaCharacter* UArenaCharacterState::GetMyPawn() const
{
	return MyPawn;
}
void UArenaCharacterState::SetMyPawn(AArenaCharacter* Pawn)
{
	MyPawn = Pawn;
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
void UArenaCharacterState::SetCombatState(ECombatState::Type NewState, class AArenaCharacter* Pawn)
{
	CombatState = NewState;
}

///////////////////////////////////////////// Replication /////////////////////////////////////////////

void UArenaCharacterState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UArenaCharacterState, PlayerState);
	DOREPLIFETIME(UArenaCharacterState, CombatState);
}

void UArenaCharacterState::OnRep_PlayerState()
{

}

void UArenaCharacterState::OnRep_CombatState(ECombatState::Type NewState)
{
	if (CombatState == ECombatState::Aggressive)
	{
		//Pawn->EnterCombat();
	}
}