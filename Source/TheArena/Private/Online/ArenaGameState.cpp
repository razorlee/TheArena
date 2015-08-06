// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaGameState.h"


AArenaGameState::AArenaGameState(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	NumTeams = 0;
	RemainingTime = 0;
	bTimerPaused = false;
}

void AArenaGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaGameState, NumTeams);
	DOREPLIFETIME(AArenaGameState, RemainingTime);
	DOREPLIFETIME(AArenaGameState, bTimerPaused);
	DOREPLIFETIME(AArenaGameState, TeamScores);
}

void AArenaGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
{
	OutRankedMap.Empty();

	//first, we need to go over all the PlayerStates, grab their score, and rank them
	TMultiMap<int32, AArenaPlayerState*> SortedMap;
	for (int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		int32 Score = 0;
		AArenaPlayerState* CurPlayerState = Cast<AArenaPlayerState>(PlayerArray[i]);
		if (CurPlayerState && (1.0f == TeamIndex))
		{
			SortedMap.Add(FMath::TruncToInt(CurPlayerState->Score), CurPlayerState);
		}
	}

	//sort by the keys
	SortedMap.KeySort(TGreater<int32>());

	//now, add them back to the ranked map
	OutRankedMap.Empty();

	int32 Rank = 0;
	for (TMultiMap<int32, AArenaPlayerState*>::TIterator It(SortedMap); It; ++It)
	{
		OutRankedMap.Add(Rank++, It.Value());
	}

}

void AArenaGameState::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		ATheArenaGameMode* const GameMode = Cast<ATheArenaGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle our own business
		/*UArenaGameInstance* GI = Cast<UArenaGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->RemoveSplitScreenPlayers();
		}

		APlayerController* const PrimaryPC = GetGameInstance()->GetFirstLocalPlayerController();
		if (PrimaryPC)
		{
			check(PrimaryPC->GetNetMode() == ENetMode::NM_Client);
			PrimaryPC->ClientReturnToMainMenu(TEXT("Return to Main Menu requested by game state"));
		}*/
	}

}
