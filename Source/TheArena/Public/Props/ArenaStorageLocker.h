// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Props/ArenaInteractiveObject.h"
#include "ArenaStorageLocker.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaStorageLocker : public AArenaInteractiveObject
{
	GENERATED_BODY()

public:

	// Constructor to set defaults
	AArenaStorageLocker();

	// Override on interact event
	void OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate = 1.f) override;

	// Override on view event
	void OnView_Implementation(AArenaCharacter* Player) override;

	// Override on leave event
	void OnLeave_Implementation(AArenaCharacter* Player) override;

private:
	
};
