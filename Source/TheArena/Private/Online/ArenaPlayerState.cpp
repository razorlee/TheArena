// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaPlayerState.h"


AArenaPlayerState::AArenaPlayerState(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
}

void AArenaPlayerState::Reset()
{
	Super::Reset();

	SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
}

void AArenaPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void AArenaPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

void AArenaPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AArenaPlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void AArenaPlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void AArenaPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AArenaCharacter* ArenaCharacter = Cast<AArenaCharacter>(OwnerController->GetCharacter());
		if (ArenaCharacter != NULL)
		{
			ArenaCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}

int32 AArenaPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

int32 AArenaPlayerState::GetKills() const
{
	return NumKills;
}

int32 AArenaPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float AArenaPlayerState::GetScore() const
{
	return Score;
}

int32 AArenaPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 AArenaPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

void AArenaPlayerState::ScoreKill(AArenaPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AArenaPlayerState::ScoreDeath(AArenaPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void AArenaPlayerState::ScorePoints(int32 Points)
{
	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GetWorld()->GameState);
	if (MyGameState && TeamNumber >= 0)
	{
		if (TeamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[TeamNumber] += Points;
	}

	Score += Points;
}

void AArenaPlayerState::InformAboutKill_Implementation(class AArenaPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AArenaPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AArenaPlayerController* TestPC = Cast<AArenaPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				/*TSharedPtr<FUniqueNetId> LocalID = LocalPlayer->GetUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					TestPC->OnKill();
				}*/
			}
		}
	}
}

void AArenaPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaPlayerState, TeamNumber);
	DOREPLIFETIME(AArenaPlayerState, NumKills);
	DOREPLIFETIME(AArenaPlayerState, NumDeaths);
}

FString AArenaPlayerState::GetShortPlayerName() const
{
	if (PlayerName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		return PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}
	return PlayerName;
}

