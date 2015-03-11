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

	/** Sets the online mode of the game */
	void SetIsOnline(bool bInIsOnline);

	bool HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL);

	bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults);
	bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult);

	/** Initiates the session searching */
	bool FindSessions(ULocalPlayer* PlayerOwner, bool bLANMatch);

	/** Returns true if the game is in online mode */
	bool GetIsOnline() const { return bIsOnline; }
	bool HasLicense() const { return bIsLicensed; }

	/** Sends the game to the specified state. */
	void GotoState(FName NewState);

	/** Start task to get user privileges. */
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< FUniqueNetId > UserId);

	/** Common cleanup code for any Privilege task delegate */
	void CleanupOnlinePrivilegeTask();

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
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is called after adding local users to a session we're joining */
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Whether the match is online or not */
	bool bIsOnline;

	/** Whether the user has an active license to play the game */
	bool bIsLicensed;

protected:
	
};

