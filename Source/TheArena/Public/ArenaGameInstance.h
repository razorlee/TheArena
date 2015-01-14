// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TheArena.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "ArenaGameInstance.generated.h"

class FVariantData;
namespace ArenaGameInstanceState
{
	extern const FName None;
	extern const FName PendingInvite;
	extern const FName WelcomeScreen;
	extern const FName MainMenu;
	extern const FName MessageMenu;
	extern const FName Playing;
}

UCLASS(config = Game)
class UArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UArenaGameInstance(const FObjectInitializer& ObjectInitializer);

public:

	class AArenaGameSession* GetGameSession() const;

	bool HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL);
	bool JoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** Returns true if the game is in online mode */
	bool GetIsOnline() const { return bIsOnline; }
	bool HasLicense() const { return bIsLicensed; }

	/** Sends the game to the specified state. */
	void GotoState(FName NewState);

private:
	FName CurrentState;
	FName PendingState;

	void AddNetworkFailureHandlers();
	void RemoveNetworkFailureHandlers();

	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/** Travel directly to the named session */
	void InternalTravelToSession(const FName& SessionName);

	/** Called after all the local players are registered */
	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Whether the match is online or not */
	bool bIsOnline;

	/** Whether the user has an active license to play the game */
	bool bIsLicensed;

protected:
	
};

