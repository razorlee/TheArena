// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaTeamSlayer.h"

#include "TheArena.h"

AArenaTeamSlayer::AArenaTeamSlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NumTeams = 2;
	bDelayedStart = false;
	//TeamBalance.AddZeroed(NumTeams);
}

void AArenaTeamSlayer::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	AArenaPlayerState* NewPlayerState = CastChecked<AArenaPlayerState>(NewPlayer->PlayerState);
	const int32 TeamNum = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeamNum(TeamNum);

	Super::PostLogin(NewPlayer);
}

void AArenaTeamSlayer::InitGameState()
{
	Super::InitGameState();

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->NumTeams = NumTeams;
	}
}

bool AArenaTeamSlayer::CanDealDamage(class AArenaPlayerState* DamageInstigator, class AArenaPlayerState* DamagedPlayer) const
{
	return true;
}

int32 AArenaTeamSlayer::ChooseTeam(AArenaPlayerState* ForPlayerState) const
{
	TArray<int32> TeamBalance;
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

	// find least populated one
	int32 BestTeamScore = TeamBalance[0];
	for (int32 i = 1; i < TeamBalance.Num(); i++)
	{
		if (BestTeamScore > TeamBalance[i])
		{
			BestTeamScore = TeamBalance[i];
		}
	}

	// there could be more than one...
	TArray<int32> BestTeams;
	for (int32 i = 0; i < TeamBalance.Num(); i++)
	{
		if (TeamBalance[i] == BestTeamScore)
		{
			BestTeams.Add(i);
		}
	}

	// get random from best list
	const int32 RandomBestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	return RandomBestTeam;
}

void AArenaTeamSlayer::DetermineMatchWinner()
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
		else if (BestScore == TeamScore)
		{
			NumBestTeams++;
		}
	}

	WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;
}

bool AArenaTeamSlayer::IsWinner(class AArenaPlayerState* PlayerState) const
{
	return PlayerState && PlayerState->GetTeamNum() == WinnerTeam;
}

bool AArenaTeamSlayer::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
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

