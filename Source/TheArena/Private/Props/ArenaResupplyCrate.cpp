// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaResupplyCrate.h"
#include "Kismet/GameplayStatics.h"
#include "ArenaCharacter.h"


// Set Default values
AArenaResupplyCrate::AArenaResupplyCrate()
{
	AmmoResupply = 50;

	// 20 Second delay
	DelayLength = 20.f;
}

void AArenaResupplyCrate::OnInteractEnd_Implementation()
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	// Add ammo to player's primary and secondary gun
	if (InteractedPlayer)
	{
		InteractedPlayer->GetPrimaryWeapon()->GetWeaponAttributes()->TotalAmmo += AmmoResupply;
		InteractedPlayer->GetSecondaryWeapon()->GetWeaponAttributes()->TotalAmmo += AmmoResupply;

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Ammo Added")));
	}

	Super::OnInteractEnd_Implementation();
	
}

int32 AArenaResupplyCrate::GetAmmoResupply()
{
	return AmmoResupply;
}

void AArenaResupplyCrate::SetAmmoResupply(int32 resupplyAmount)
{
	AmmoResupply = resupplyAmount;
}

float AArenaResupplyCrate::GetResupplyDelay()
{
	return DelayLength;
}

void AArenaResupplyCrate::SetResupplyDelay(float resupplyDelay)
{
	DelayLength = resupplyDelay;
}