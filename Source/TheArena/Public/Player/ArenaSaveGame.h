// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "ArenaSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API UArenaSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TSubclassOf<class AArenaWeapon> PrimaryWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TSubclassOf<class AArenaWeapon> SecondaryWeapon;

	UArenaSaveGame(const FObjectInitializer& ObjectInitializer);
	
};
