// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "TheArena.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "Blueprint/UserWidget.h"
#include "ArenaStartMenu.generated.h"

/**
 * 
 */
UCLASS()
class UArenaStartMenu : public UUserWidget
{
	GENERATED_BODY()

public: 

	virtual ~UArenaStartMenu();

	class AArenaGameSession* GetGameSession() const;

	/** Initialize menu */
	UFUNCTION(BlueprintCallable, Category = Online)
	void SetUp(UArenaGameInstance* _GameInstance, ULocalPlayer* _PlayerOwner);

	/** Returns the player that owns the main menu. */
	ULocalPlayer* GetPlayerOwner() const;

	/** Returns the string name of the currently selected map */
	FString GetMapName() const;

	/** Hosts team deathmatch game */
	UFUNCTION(BlueprintCallable, Category = Online)
	void HostTeamDeathMatch();

	/** Hosts a game, using the passed in game type */
	UFUNCTION(BlueprintCallable, Category = Online)
	void HostGame(const FString& GameType);

protected:

	/** Owning player */
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;

	/** Owning game instance */
	TWeakObjectPtr<UArenaGameInstance> GameInstance;

	/** lan game? */
	bool bIsLanMatch;
	
};
