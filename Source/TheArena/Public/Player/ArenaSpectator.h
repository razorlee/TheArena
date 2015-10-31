// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "ArenaSpectator.generated.h"

UCLASS(config = Game, Blueprintable, BlueprintType)
class THEARENA_API AArenaSpectator : public ASpectatorPawn
{
	GENERATED_UCLASS_BODY()
	
	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;
	/** Overridden to implement Key Bindings the match the player controls */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Frame rate linked look
	void LookUpAtRate(float Val);
	/** player pressed climb action */
	void OnNextCamera();
	/** player pressed climb action */
	void OnPreviousCamera();

	int32 index;

	AArenaCharacter* FollowPawn;

	AController* OriginalController;
};