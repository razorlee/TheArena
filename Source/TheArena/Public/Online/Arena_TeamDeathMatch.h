// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arena_TeamDeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArena_TeamDeathMatch : public ATheArenaGameMode
{
	GENERATED_BODY()

	AArena_TeamDeathMatch(const class FObjectInitializer& PCIP);

	/** initialize player */
	virtual void InitNewPlayer(APlayerController* NewPlayer, const TSharedPtr<FUniqueNetId>& UniqueId, const FString& Options);

	/** initialize replicated game data */
	virtual void InitGameState() override;

	/** can players damage each other? */
	virtual bool CanDealDamage(class AArenaPlayerState* DamageInstigator, class AArenaPlayerState* DamagedPlayer) const override;
	
protected:

	/** number of teams */
	int32 NumTeams;

	/** best team */
	int32 WinnerTeam;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(class AArenaPlayerState* ForPlayerState) const;

	/** check who won */
	virtual void DetermineMatchWinner() override;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class AArenaPlayerState* PlayerState) const override;

	/** check team constraints */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

};
