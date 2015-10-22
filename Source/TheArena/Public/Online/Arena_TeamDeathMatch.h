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

	AArena_TeamDeathMatch(const class FObjectInitializer& ObjectInitializer);

	/** initialize player */
	void PostLogin(APlayerController* NewPlayer) override;

	/** initialize replicated game data */
	virtual void InitGameState() override;

	/** can players damage each other? */
	virtual bool CanDealDamage(class AArenaPlayerState* DamageInstigator, class AArenaPlayerState* DamagedPlayer) const override;
	
protected:

	/** number of teams */
	int32 NumTeams;
	
	/** round counter */
	int32 Round;

	/** best team */
	int32 WinnerTeam;

	/** team balance */
	TArray<int32> TeamBalance;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(class AArenaPlayerState* ForPlayerState);

	/** check who won */
	virtual void DetermineMatchWinner() override;

	/** check if won */
	virtual void CheckTeamElimination() override;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class AArenaPlayerState* PlayerState) const override;

	/** check team constraints */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

};
