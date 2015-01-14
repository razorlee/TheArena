#include "TheArena.h"

#include "ArenaMainMenu.h"
#include "OnlinePartyInterface.h"
#include "OnlineKeyValuePair.h"

namespace ArenaGameInstanceState
{
	const FName None = FName(TEXT("None"));
	const FName PendingInvite = FName(TEXT("PendingInvite"));
	const FName WelcomeScreen = FName(TEXT("WelcomeScreen"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName MessageMenu = FName(TEXT("MessageMenu"));
	const FName Playing = FName(TEXT("Playing"));
}

UArenaGameInstance::UArenaGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsOnline(true) // Default to online
	, bIsLicensed(true) // Default to licensed (should have been checked by OS on boot)
{
	CurrentState = ArenaGameInstanceState::None;
}

bool UArenaGameInstance::HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL)
{
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		// add callback delegate for completion
		GameSession->OnCreatePresenceSessionComplete().AddUObject(this, &UArenaGameInstance::OnCreatePresenceSessionComplete);

		TravelURL = InTravelURL;
		bool const bIsLanMatch = InTravelURL.Contains(TEXT("?bIsLanMatch"));

		//determine the map name from the travelURL
		const FString& MapNameSubStr = "/Game/Levels/";
		const FString& ChoppedMapName = TravelURL.RightChop(MapNameSubStr.Len());
		const FString& MapName = ChoppedMapName.LeftChop(ChoppedMapName.Len() - ChoppedMapName.Find("?game"));

		if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, GameType, MapName, bIsLanMatch, true, AArenaGameSession::DEFAULT_NUM_PLAYERS))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ArenaGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				//ShowLoadingScreen();
				GotoState(ArenaGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

bool UArenaGameInstance::JoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// needs to tear anything down based on current state?
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		AddNetworkFailureHandlers();
		FOnJoinSessionCompleteDelegate joinDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UArenaGameInstance::OnJoinSessionCompleted);

		/*GameSession->OnJoinSessionComplete().AddUObject(this, &UArenaGameInstance::OnJoinSessionComplete);

		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SearchResult))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ArenaGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				//ShowLoadingScreen();
				GotoState(ArenaGameInstanceState::Playing);
				return true;
			}
		}*/
	}

	return false;
}

void UArenaGameInstance::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	AArenaPlayerController_Menu* const FirstPC = Cast<AArenaPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (FirstPC != nullptr)
	{
		FString ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.").ToString();
		if (ReasonString.IsEmpty() == false)
		{
			ReturnReason += " ";
			ReturnReason += ReasonString;
		}

		//FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FString());
	}
}

void UArenaGameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogOnline, Verbose, TEXT("JoinSessionCompleted"));
	IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
	if (Sessions.IsValid())
	{
		UE_LOG(LogOnline, Verbose, TEXT("Sessions Valid"));
		if (Result == EOnJoinSessionCompleteResult::Success)
		{
			// Client travel to the server
			FString ConnectString;
			if (Sessions->GetResolvedConnectString(GameSessionName, ConnectString))
			{
				UE_LOG(LogOnline, Log, TEXT("Join session: traveling to %s"), *ConnectString);
				GetFirstLocalPlayerController()->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}
}

void UArenaGameInstance::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		FString ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::RoomIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game is full.").ToString();
			break;
		case EOnJoinSessionCompleteResult::RoomDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game no longer exists.").ToString();
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.").ToString();
			break;
		}

		FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
		RemoveNetworkFailureHandlers();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FString());
		return;
	}

	//InternalTravelToSession(GameSessionName);
}

void UArenaGameInstance::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->OnCreatePresenceSessionComplete().RemoveUObject(this, &UArenaGameInstance::OnCreatePresenceSessionComplete);

		// We either failed or there is only a single local user
		FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UArenaGameInstance::FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Travel to the specified match URL
		GetWorld()->ServerTravel(TravelURL);
	}
	else
	{
		FString ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.").ToString();
		//FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FString());
	}
}

AArenaGameSession* UArenaGameInstance::GetGameSession() const
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

void UArenaGameInstance::GotoState(FName NewState)
{
	UE_LOG(LogOnline, Log, TEXT("GotoState: NewState: %s"), *NewState.ToString());

	PendingState = NewState;
}

void UArenaGameInstance::RemoveNetworkFailureHandlers()
{
	// Remove the local session/travel failure bindings if they exist
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnTravelFailure().RemoveUObject(this, &UArenaGameInstance::TravelLocalSessionFailure);
	}
}

void UArenaGameInstance::AddNetworkFailureHandlers()
{
	// Add network/travel error handlers (if they are not already there)
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
	{
		GEngine->OnTravelFailure().AddUObject(this, &UArenaGameInstance::TravelLocalSessionFailure);
	}
}





