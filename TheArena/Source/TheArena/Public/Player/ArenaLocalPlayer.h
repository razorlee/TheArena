// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LocalPlayer.h"
#include "ArenaLocalPlayer.generated.h"

UCLASS(config=game, transient)
class THEARENA_API UArenaLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

public:

	virtual void SetControllerId(int32 NewControllerId) override;

	//class UShooterPersistentUser* GetPersistentUser() const;

	/** Initializes the PersistentUser */
	//void LoadPersistentUser();

private:
	/** Persistent user data stored between sessions (i.e. the user's savegame) */
	//UPROPERTY()
	//class UShooterPersistentUser* PersistentUser;

	/** @return OnlineSession class to use for this player */
	//TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
	
};
