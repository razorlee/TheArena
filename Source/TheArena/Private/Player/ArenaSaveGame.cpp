// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaSaveGame.h"


UArenaSaveGame::UArenaSaveGame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SaveSlotName = TEXT("SavedGame");
	UserIndex = 0;
	Health = 0;
}

