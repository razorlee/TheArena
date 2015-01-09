// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "Arena_Menu.h"

AArena_Menu::AArena_Menu(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	PlayerControllerClass = AArenaPlayerController_Menu::StaticClass();
}

void AArena_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AArena_Menu::GetGameSessionClass() const
{
	return AArenaGameSession::StaticClass();
}

