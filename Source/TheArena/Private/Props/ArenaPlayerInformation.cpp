// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaPlayerInformation.h"

AArenaPlayerInformation::AArenaPlayerInformation()
{

}

void AArenaPlayerInformation::OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate)
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
			//PC->OnToggleInventory();
		}
	}

}

void AArenaPlayerInformation::OnView_Implementation(AArenaCharacter* Player)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	Super::OnView_Implementation(Player);
	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Player->GetController());
	{
		//PC->SetNearbyInventory(true);
	}
}

void AArenaPlayerInformation::OnLeave_Implementation(AArenaCharacter* Player)
{
	Super::OnLeave_Implementation(Player);
	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Player->GetController());
	{
		//PC->SetNearbyInventory(false);
	}
}


