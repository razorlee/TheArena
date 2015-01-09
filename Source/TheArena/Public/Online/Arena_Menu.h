// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "Arena_Menu.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArena_Menu : public AGameMode
{
	GENERATED_BODY()

	AArena_Menu(const class FObjectInitializer& PCIP);
	
public:

	// Begin AGameMode interface
	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) override;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
	// End AGameMode interface

protected:

	/** Perform some final tasks before hosting/joining a session. Remove menus, set king state etc */
	void BeginSession();

	/** Display a loading screen */
	void ShowLoadingScreen();
	
};
