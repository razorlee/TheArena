// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaMatchmakingTerminal.h"

AArenaMatchmakingTerminal::AArenaMatchmakingTerminal()
{

}

void AArenaMatchmakingTerminal::OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	Super::OnInteract_Implementation(Player, InPlayRate);

	if (Player)
	{
		AArenaPlayerController* PC = Cast<AArenaPlayerController>(Player->GetController());
		if (PC)
		{
			PC->OnToggleMatchmaking();
		}
	}

}

void AArenaMatchmakingTerminal::OnView_Implementation(AArenaCharacter* Player)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	Super::OnView_Implementation(Player);
	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Player->GetController());
	{
		PC->SetNearbyMatchmaking(true);
	}
}

void AArenaMatchmakingTerminal::OnLeave_Implementation(AArenaCharacter* Player)
{
	Super::OnLeave_Implementation(Player);
	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Player->GetController());
	{
		PC->SetNearbyMatchmaking(false);
	}
}


