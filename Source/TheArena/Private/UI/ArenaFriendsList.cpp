// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "Networking.h"
#include "ArenaFriendsList.h"
#include "OnlineSubsystemUtils.h"
#include "ArenaGameInstance.h"

#define LOCTEXT_NAMESPACE "ArenaGame.HUD.Menu"

void UArenaFriendsList::SetUp(UArenaGameInstance* _GameInstance, ULocalPlayer* _PlayerOwner)
{
	PlayerOwner = _PlayerOwner;
	GameInstance = _GameInstance;
	MapFilterName = "Any";
	bSearchingForServers = true;
	bLANMatchSearch = false;
	StatusText = FString();
}

/** Updates current search status */
void UArenaFriendsList::UpdateSearchStatus()
{
	check(bSearchingForServers); // should not be called otherwise

	bool bFinishSearch = true;
	AArenaGameSession* ArenaSession = GetGameSession();
	if (ArenaSession)
	{
		int32 CurrentSearchIdx, NumSearchResults;
		EOnlineAsyncTaskState::Type SearchState = ArenaSession->GetSearchResultStatus(CurrentSearchIdx, NumSearchResults);

		UE_LOG(LogOnlineGame, Log, TEXT("ArenaSession->GetSearchResultStatus: %s"), EOnlineAsyncTaskState::ToString(SearchState));

		switch (SearchState)
		{
		case EOnlineAsyncTaskState::InProgress:
			StatusText = LOCTEXT("Searching", "SEARCHING...").ToString();
			bFinishSearch = false;
			break;

		case EOnlineAsyncTaskState::Done:
			// copy the results
		{
			ServerList.Empty();
			const TArray<FOnlineSessionSearchResult> & SearchResults = ArenaSession->GetSearchResults();
			check(SearchResults.Num() == NumSearchResults);
			if (NumSearchResults == 0)
			{
				StatusText = LOCTEXT("NoServersFound", "NO FRIENDS ONLINE, PRESS SPACE TO REFRESH").ToString();
			}
			else
			{
				StatusText = LOCTEXT("ServersRefresh", "PRESS SPACE TO REFRESH").ToString();
			}

			for (int32 IdxResult = 0; IdxResult < NumSearchResults; ++IdxResult)
			{
				TSharedPtr<FServerEntry> NewServerEntry = MakeShareable(new FServerEntry());

				const FOnlineSessionSearchResult& Result = SearchResults[IdxResult];

				NewServerEntry->ServerName = Result.Session.OwningUserName;
				NewServerEntry->Ping = FString::FromInt(Result.PingInMs);
				NewServerEntry->CurrentPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
					+ Result.Session.SessionSettings.NumPrivateConnections
					- Result.Session.NumOpenPublicConnections
					- Result.Session.NumOpenPrivateConnections);
				NewServerEntry->MaxPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
					+ Result.Session.SessionSettings.NumPrivateConnections);
				NewServerEntry->SearchResultsIndex = IdxResult;

				//Result.Session.SessionSettings.Get(SETTING_GAMEMODE, NewServerEntry->GameType);
				//Result.Session.SessionSettings.Get(SETTING_MAPNAME, NewServerEntry->MapName);

				ServerList.Add(NewServerEntry);
			}
		}
		break;

		case EOnlineAsyncTaskState::Failed:
			// intended fall-through
		case EOnlineAsyncTaskState::NotStarted:
			StatusText = "";
			// intended fall-through
		default:
			break;
		}
	}

	if (bFinishSearch)
	{
		OnServerSearchFinished();
	}
}

void UArenaFriendsList::BeginServerSearch(bool bLANMatch, const FString& InMapFilterName)
{
	bLANMatchSearch = bLANMatch;
	MapFilterName = InMapFilterName;
	bSearchingForServers = true;
	ServerList.Empty();

	UArenaGameInstance* const GI = Cast<UArenaGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		GI->FindSessions(PlayerOwner.Get(), bLANMatchSearch);
	}
}

/** Called when server search is finished */
void UArenaFriendsList::OnServerSearchFinished()
{
	bSearchingForServers = false;

	UpdateServerList();
}


void UArenaFriendsList::ConnectToServer()	
{
	if (bSearchingForServers)
	{
		// unsafe
		return;
	}
#if WITH_EDITOR
	if (GIsEditor == true)
	{
		return;
	}
#endif
	if (SelectedItem.IsValid())
	{
		int ServerToJoin = SelectedItem->SearchResultsIndex;

		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveAllViewportWidgets();
		}

		UArenaGameInstance* const GI = Cast<UArenaGameInstance>(PlayerOwner->GetGameInstance());
		if (GI)
		{
			GI->JoinSession(PlayerOwner.Get(), ServerToJoin);
		}
	}
}

void UArenaFriendsList::UpdateServerList()
{
	/** Only filter maps if a specific map is specified */
	if (MapFilterName != "Any")
	{
		for (int32 i = 0; i < ServerList.Num(); ++i)
		{
			/** Only filter maps if a specific map is specified */
			/*if (ServerList[i]->MapName != MapFilterName)
			{
				ServerList.RemoveAt(i);
			}*/
		}
	}

	int32 SelectedItemIndex = ServerList.IndexOfByKey(SelectedItem);
	
	/*ServerListWidget->RequestListRefresh();
	if (ServerList.Num() > 0)
	{
		ServerListWidget->UpdateSelectionSet();
		ServerListWidget->SetSelection(ServerList[SelectedItemIndex > -1 ? SelectedItemIndex : 0], ESelectInfo::OnNavigation);
	}*/

}

AArenaGameSession* UArenaFriendsList::GetGameSession() const
{
	UArenaGameInstance* const GI = Cast<UArenaGameInstance>(PlayerOwner->GetGameInstance());
	return GI ? GI->GetGameSession() : nullptr;
}

TArray<FServerEntry> UArenaFriendsList::GetServerList()
{
	TArray<FServerEntry> servers;
	for (int i = 0; i < ServerList.Num(); i++)
	{
		servers.Add(*ServerList[i].Get());
	}
	return  servers;
}

void UArenaFriendsList::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	if (GameInstance.IsValid())
	{
		// Lock controls for the duration of the async task
		//MenuWidget->LockControls(true);//recall
		TSharedPtr<FUniqueNetId> UserId;
		if (PlayerOwner.IsValid())
		{
			UserId = PlayerOwner->GetPreferredUniqueNetId();
		}
		GameInstance->StartOnlinePrivilegeTask(Delegate, EUserPrivileges::CanPlayOnline, UserId);
	}
}

void UArenaFriendsList::OnJoinServer()
{
	StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UArenaFriendsList::OnUserCanPlayOnlineJoin));
}

void UArenaFriendsList::OnUserCanPlayOnlineJoin(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	//MenuWidget->LockControls(false);

	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{

		//make sure to switch to custom match type so we don't instead use Quick type
		//MatchType = EMatchType::Custom;

		if (GameInstance.IsValid())
		{
			GameInstance->SetIsOnline(true);
		}

		//MatchType = EMatchType::Custom;

		//MenuWidget->NextMenu = JoinServerItem->SubMenu;
		//FString SelectedMapFilterName = JoinMapOption->MultiChoice[JoinMapOption->SelectedMultiChoice].ToString();

		BeginServerSearch(false, TEXT("ANY"));
		UpdateServerList();
		//MenuWidget->EnterSubMenu();
	}
	else if (GameInstance.IsValid())
	{
		//GameInstance->DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
	}
}

void UArenaFriendsList::CleanupOnlinePrivilegeTask()
{
	if (GameInstance.IsValid())
	{
		GameInstance->CleanupOnlinePrivilegeTask();
	}
}

void UArenaFriendsList::Search()
{
	if (bSearchingForServers)
	{
		UpdateSearchStatus();
	}
}

void UArenaFriendsList::EntrySelectionChanged(int32 index)
{
	SelectedItem = ServerList[index];
}

#undef LOCTEXT_NAMESPACE