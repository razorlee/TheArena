// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaGameSession.h"

namespace
{
	const FString CustomMatchKeyword("Custom");
}

AArenaGameSession::AArenaGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &AArenaGameSession::OnCreateSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &AArenaGameSession::OnDestroySessionComplete);

		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &AArenaGameSession::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &AArenaGameSession::OnJoinSessionComplete);

		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &AArenaGameSession::OnStartOnlineGameComplete);
	}
}

/**
* Delegate fired when a session start request has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaGameSession::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			//Sessions->ClearOnStartSessionCompleteDelegate(OnStartSessionCompleteDelegate);
		}
	}
	if (bWasSuccessful)
	{
		// tell non-local players to start online game
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
			if (PC && !PC->IsLocalPlayerController())
			{
				PC->ClientStartOnlineGame();
			}
		}
	}
}

/** Handle starting the match */
void AArenaGameSession::HandleMatchHasStarted()
{
	// start online game locally and wait for completion
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			UE_LOG(LogOnlineGame, Log, TEXT("Starting session %s on server"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			Sessions->StartSession(GameSessionName);
		}
	}
}

/**
* Ends a game session
*
*/
void AArenaGameSession::HandleMatchHasEnded()
{
	// start online game locally and wait for completion
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// tell the clients to end
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
				if (PC && !PC->IsLocalPlayerController())
				{
					PC->ClientEndOnlineGame();
				}
			}

			// server is handled here
			UE_LOG(LogOnlineGame, Log, TEXT("Ending session %s on server"), *GameSessionName.ToString());
			Sessions->EndSession(GameSessionName);
		}
	}
}

bool AArenaGameSession::IsBusy() const
{
	if (HostSettings.IsValid() || SearchSettings.IsValid())
	{
		return true;
	}

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type GameSessionState = Sessions->GetSessionState(GameSessionName);
			EOnlineSessionState::Type PartySessionState = Sessions->GetSessionState(PartySessionName);
			if (GameSessionState != EOnlineSessionState::NoSession || PartySessionState != EOnlineSessionState::NoSession)
			{
				return true;
			}
		}
	}

	return false;
}

EOnlineAsyncTaskState::Type AArenaGameSession::GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults)
{
	SearchResultIdx = 0;
	NumSearchResults = 0;

	if (SearchSettings.IsValid())
	{
		if (SearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			SearchResultIdx = CurrentSessionParams.BestSessionIdx;
			NumSearchResults = SearchSettings->SearchResults.Num();
		}
		return SearchSettings->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

/**
* Get the search results.
*
* @return Search results
*/
const TArray<FOnlineSessionSearchResult> & AArenaGameSession::GetSearchResults() const
{
	return SearchSettings->SearchResults;
};


/**
* Delegate fired when a session create request has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaGameSession::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnCreateSessionComplete %s bSuccess: %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	OnCreatePresenceSessionComplete().Broadcast(SessionName, bWasSuccessful);
}

/**
* Delegate fired when a destroying an online session has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaGameSession::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnDestroySessionComplete %s bSuccess: %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		HostSettings = NULL;
	}
}

bool AArenaGameSession::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FString& GameType, const FString& MapName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		CurrentSessionParams.SessionName = SessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;
		MaxPlayers = MaxNumPlayers;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		//if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		{
			HostSettings = MakeShareable(new FArenaOnlineSessionSettings(bIsLAN, bIsPresence, MaxNumPlayers));
			HostSettings->Set(SETTING_GAMEMODE, GameType, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			return Sessions->CreateSession(*CurrentSessionParams.UserId, CurrentSessionParams.SessionName, *HostSettings);
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		// Hack workflow in development
		OnCreatePresenceSessionComplete().Broadcast(GameSessionName, true);
		return true;
	}
#endif

	return false;
}

void AArenaGameSession::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("OnFindSessionsComplete bSuccess: %d"), bWasSuccessful);

	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			UE_LOG(LogOnlineGame, Verbose, TEXT("Num Search Results: %d"), SearchSettings->SearchResults.Num());
			for (int32 SearchIdx = 0; SearchIdx < SearchSettings->SearchResults.Num(); SearchIdx++)
			{
				const FOnlineSessionSearchResult& SearchResult = SearchSettings->SearchResults[SearchIdx];
				DumpSession(&SearchResult.Session);
			}

			OnFindSessionsComplete().Broadcast(bWasSuccessful);
		}
	}
}

void AArenaGameSession::ResetBestSessionVars()
{
	CurrentSessionParams.BestSessionIdx = -1;
}

void AArenaGameSession::ChooseBestSession()
{
	// Start searching from where we left off
	for (int32 SessionIndex = CurrentSessionParams.BestSessionIdx + 1; SessionIndex < SearchSettings->SearchResults.Num(); SessionIndex++)
	{
		// Found the match that we want
		CurrentSessionParams.BestSessionIdx = SessionIndex;
		return;
	}

	CurrentSessionParams.BestSessionIdx = -1;
}

void AArenaGameSession::StartMatchmaking()
{
	ResetBestSessionVars();
	ContinueMatchmaking();
}

void AArenaGameSession::ContinueMatchmaking()
{
	ChooseBestSession();
	if (CurrentSessionParams.BestSessionIdx >= 0 && CurrentSessionParams.BestSessionIdx < SearchSettings->SearchResults.Num())
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
			{
				OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
				Sessions->JoinSession(*CurrentSessionParams.UserId, CurrentSessionParams.SessionName, SearchSettings->SearchResults[CurrentSessionParams.BestSessionIdx]);
			}
		}
	}
	else
	{
		OnNoMatchesAvailable();
	}
}

void AArenaGameSession::OnNoMatchesAvailable()
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("Matchmaking complete, no sessions available."));
	SearchSettings = NULL;
}

void AArenaGameSession::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		CurrentSessionParams.SessionName = SessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		{
			SearchSettings = MakeShareable(new FArenaOnlineSearchSettings(bIsLAN, bIsPresence));
			SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineComparisonOp::Equals);

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			Sessions->FindSessions(*CurrentSessionParams.UserId, SearchSettingsRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

bool AArenaGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, int32 SessionIndexInSearchResults)
{
	bool bResult = false;

	if (SessionIndexInSearchResults >= 0 && SessionIndexInSearchResults < SearchSettings->SearchResults.Num())
	{
		bResult = JoinSession(UserId, SessionName, SearchSettings->SearchResults[SessionIndexInSearchResults]);
	}

	return bResult;
}

bool AArenaGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	bool bResult = false;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
			bResult = Sessions->JoinSession(*UserId, SessionName, SearchResult);
		}
	}

	return bResult;
}

/**
* Delegate fired when the joining process for an online session has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaGameSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bWillTravel = false;

	UE_LOG(LogOnlineGame, Verbose, TEXT("OnJoinSessionComplete %s bSuccess: %d"), *SessionName.ToString(), static_cast<int32>(Result));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = NULL;
	if (OnlineSub)
	{
		Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	OnJoinSessionComplete().Broadcast(Result);
}

bool AArenaGameSession::TravelToSession(int32 ControllerId, FName SessionName)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		FString URL;
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && Sessions->GetResolvedConnectString(SessionName, URL))
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId);
			if (PC)
			{
				PC->ClientTravel(URL, TRAVEL_Absolute);
				return true;
			}
		}
		else
		{
			UE_LOG(LogOnlineGame, Warning, TEXT("Failed to join session %s"), *SessionName.ToString());
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId);
		if (PC)
		{
			FString LocalURL(TEXT("127.0.0.1"));
			PC->ClientTravel(LocalURL, TRAVEL_Absolute);
			return true;
		}
	}
#endif //!UE_BUILD_SHIPPING

	return false;
}
