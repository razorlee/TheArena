// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaStartMenu.h"
#include "ArenaGameInstance.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "GenericPlatformChunkInstall.h"
#include "Online/ArenaOnlineGameSettings.h"

#define LOCTEXT_NAMESPACE "TheArena.UMG.Menu"

static const FString MapNames[] = { TEXT("Testing") };

UArenaStartMenu::~UArenaStartMenu()
{
	auto Sessions = Online::GetSessionInterface();
	//CleanupOnlinePrivilegeTask();
}

void UArenaStartMenu::SetUp(UArenaGameInstance* _GameInstance, ULocalPlayer* _PlayerOwner)
{
	bIsLanMatch = false;
	PlayerOwner = _PlayerOwner;
}

void UArenaStartMenu::HostGame(const FString& GameType)
{
	AArenaGameSession* const GameSession = GetGameSession();
	GameSession->HostSession(PlayerOwner->GetPreferredUniqueNetId(), GameSessionName, "TDM", "Testing", false, true, 12);
}

void UArenaStartMenu::HostTeamDeathMatch()
{
	HostGame(LOCTEXT("TDM", "TDM").ToString());
}

ULocalPlayer* UArenaStartMenu::GetPlayerOwner() const
{
	return PlayerOwner.Get();
}

FString UArenaStartMenu::GetMapName() const
{
	return MapNames[0];
}

AArenaGameSession* UArenaStartMenu::GetGameSession() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameMode* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<AArenaGameSession>(Game->GameSession);
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE

