// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"

AArena_TeamDeathMatch::AArena_TeamDeathMatch(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumTeams = 2;
	Round = 0;
	bDelayedStart = true;
}

void AArena_TeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	AArenaPlayerState* NewPlayerState = CastChecked<AArenaPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNum(TeamNum);

	Super::PostLogin(NewPlayer);
}

void AArena_TeamDeathMatch::InitGameState()
{
	Super::InitGameState();

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (MyGameState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Starting Round: %d"), Round));
		MyGameState->NumTeams = NumTeams;
	}
}

bool AArena_TeamDeathMatch::CanDealDamage(class AArenaPlayerState* DamageInstigator, class AArenaPlayerState* DamagedPlayer) const
{
	return true;//DamageInstigator && DamagedPlayer && (DamagedPlayer == DamageInstigator || DamagedPlayer->GetTeamNum() != DamageInstigator->GetTeamNum());
}

int32 AArena_TeamDeathMatch::ChooseTeam(AArenaPlayerState* ForPlayerState)
{
	TeamBalance.AddZeroed(NumTeams);

	// get current team balance
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AArenaPlayerState const* const TestPlayerState = Cast<AArenaPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNum()))
		{
			TeamBalance[TestPlayerState->GetTeamNum()]++;
		}
	}

	if (TeamBalance[0] <= TeamBalance[1])
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void AArena_TeamDeathMatch::DetermineMatchWinner()
{
	AArenaGameState const* const MyGameState = Cast<AArenaGameState>(GameState);
	int32 BestScore = MAX_uint32;
	int32 BestTeam = -1;
	int32 NumBestTeams = 1;

	for (int32 i = 0; i < MyGameState->TeamScores.Num(); i++)
	{
		const int32 TeamScore = MyGameState->TeamScores[i];
		if (BestScore < TeamScore)
		{
			BestScore = TeamScore;
			BestTeam = i;
			NumBestTeams = 1;
		}
	}
	WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;

	CheckTeamElimination();
	Round++;
}

void AArena_TeamDeathMatch::CheckTeamElimination()
{
	int32 TeamOneDead = 0;
	int32 TeamTwoDead = 0;
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AArenaPlayerState const* const TestPlayerState = Cast<AArenaPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState->GetTeamNum() == 0)
		{
			AArenaCharacter *Pawn = Cast<AArenaCharacter>(TestPlayerState->GetOwner());
			if (Pawn->GetCharacterAttributes()->bIsDying)//null pointer right here, is the character already destroyed?
			{
				TeamOneDead++;
			}
		}
		if (TeamOneDead == TeamBalance[0])
		{
			WinnerTeam = 1;
			bTeamEliminated = true;
			return;
		}
	}
	for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
	{
		AArenaPlayerState const* const TestPlayerState = Cast<AArenaPlayerState>(GameState->PlayerArray[i]);
		if (TestPlayerState->GetTeamNum() == 1)
		{
			AArenaCharacter *Pawn = Cast<AArenaCharacter>(TestPlayerState->GetOwner());
			if (Pawn->GetCharacterAttributes()->bIsDying)
			{
				TeamTwoDead++;
			}
		}
		if (TeamOneDead == TeamBalance[1])
		{
			WinnerTeam = 0;
			bTeamEliminated = true;
			return;
		}
	}
}

bool AArena_TeamDeathMatch::IsWinner(class AArenaPlayerState* PlayerState) const
{
	return PlayerState && PlayerState->GetTeamNum() == WinnerTeam;
}

bool AArena_TeamDeathMatch::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	if (Player)
	{
		AArenaTeamStart* TeamStart = Cast<AArenaTeamStart>(SpawnPoint);
		AArenaPlayerState* PlayerState = Cast<AArenaPlayerState>(Player->PlayerState);

		if (PlayerState && TeamStart && TeamStart->SpawnTeam != PlayerState->GetTeamNum())
		{
			return false;
		}
	}
	return Super::IsSpawnpointAllowed(SpawnPoint, Player);
}



