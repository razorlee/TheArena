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

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> HeadUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> UpperBackUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> LowerBackUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> LeftWristUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> RightWristUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> LeftWaistUtility;

	UPROPERTY(VisibleAnywhere, Category = Utility)
	TSubclassOf<class AArenaUtility> RightWaistUtility;

	UArenaSaveGame(const FObjectInitializer& ObjectInitializer);
	
};
