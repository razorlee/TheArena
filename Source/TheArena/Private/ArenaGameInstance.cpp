#include "TheArena.h"
#include "ArenaGameInstance.h"
#include "ArenaMainMenu.h"
#include "OnlinePartyInterface.h"
#include "OnlineKeyValuePair.h"
#include "Player/ArenaPlayerController_Menu.h"
#include "Online/ArenaPlayerState.h"
#include "Online/ArenaGameSession.h"

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

void UArenaGameInstance::Init()
{
	Super::Init();

	IgnorePairingChangeForControllerId = -1;
	CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

	// game requires the ability to ID users.
	const auto OnlineSub = IOnlineSubsystem::Get();
	check(OnlineSub);
	const auto IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());

	const auto SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	// bind any OSS delegates we needs to handle
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateUObject(this, &UArenaGameInstance::HandleUserLoginChanged));
	}

	IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &UArenaGameInstance::HandleControllerPairingChanged));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UArenaGameInstance::HandleAppWillDeactivate);

	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UArenaGameInstance::HandleAppSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UArenaGameInstance::HandleAppResume);

	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UArenaGameInstance::HandleSafeFrameChanged);
	FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UArenaGameInstance::HandleControllerConnectionChange);
	FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &UArenaGameInstance::HandleAppLicenseUpdate);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UArenaGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMap.AddUObject(this, &UArenaGameInstance::OnPostLoadMap);

	FCoreUObjectDelegates::PostDemoPlay.AddUObject(this, &UArenaGameInstance::OnPostDemoPlay);

	bPendingEnableSplitscreen = false;

	OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &UArenaGameInstance::HandleNetworkConnectionStatusChanged));

	SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &UArenaGameInstance::HandleSessionFailure));

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UArenaGameInstance::OnEndSessionComplete);

	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &UArenaGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
}

void UArenaGameInstance::Shutdown()
{
	Super::Shutdown();

	// Unregister ticker delegate
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UArenaGameInstance::OnSessionUserInviteAccepted(
	const bool							bWasSuccess,
	const int32							ControllerId,
	TSharedPtr< FUniqueNetId > 			UserId,
	const FOnlineSessionSearchResult &	InviteResult
)
{
	UE_LOG(LogOnline, Verbose, TEXT("HandleSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s"), bWasSuccess, ControllerId, UserId.IsValid() ? *UserId->ToString() : TEXT("NULL "));

	if (!bWasSuccess)
	{
		return;
	}

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no search result."));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no user."));
		return;
	}

	// Set the pending invite, and then go to the initial screen, which is where we will process it
	PendingInvite.ControllerId = ControllerId;
	PendingInvite.UserId = UserId;
	PendingInvite.InviteResult = InviteResult;
	PendingInvite.bPrivilegesCheckedAndAllowed = false;

	GotoState(ArenaGameInstanceState::PendingInvite);
}

void UArenaGameInstance::HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
	UE_LOG(LogOnlineGame, Warning, TEXT("UArenaGameInstance::HandleNetworkConnectionStatusChanged: %s"), EOnlineServerConnectionStatus::ToString(ConnectionStatus));

#if ARENA_CONSOLE_UI
	// If we are disconnected from server, and not currently at (or heading to) the welcome screen
	// then display a message on consoles
	if (bIsOnline &&
		PendingState != ArenaGameInstanceState::WelcomeScreen &&
		CurrentState != ArenaGameInstanceState::WelcomeScreen &&
		ConnectionStatus != EOnlineServerConnectionStatus::Connected)
	{
		UE_LOG(LogOnlineGame, Log, TEXT("UArenaGameInstance::HandleNetworkConnectionStatusChanged: Going to main menu"));

		// Display message on consoles
#if PLATFORM_XBOXONE
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost.");
#elif PLATFORM_PS4
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to \"PSN\" has been lost.");
#else
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
#endif
		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

		ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ArenaGameInstanceState::MainMenu);
	}

	CurrentConnectionStatus = ConnectionStatus;
#endif
}

void UArenaGameInstance::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogOnlineGame, Warning, TEXT("UArenaGameInstance::HandleSessionFailure: %u"), (uint32)FailureType);

#if ARENA_CONSOLE_UI
	// If we are not currently at (or heading to) the welcome screen then display a message on consoles
	if (bIsOnline &&
		PendingState != ArenaGameInstanceState::WelcomeScreen &&
		CurrentState != ArenaGameInstanceState::WelcomeScreen)
	{
		UE_LOG(LogOnlineGame, Log, TEXT("UArenaGameInstance::HandleSessionFailure: Going to main menu"));

		// Display message on consoles
#if PLATFORM_XBOXONE
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost.");
#elif PLATFORM_PS4
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to PSN has been lost.");
#else
		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
#endif
		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

		ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ArenaGameInstanceState::MainMenu);
	}
#endif
}

void UArenaGameInstance::OnPreLoadMap()
{
	if (bPendingEnableSplitscreen)
	{
		// Allow splitscreen
		GetGameViewportClient()->SetDisableSplitscreenOverride(false);

		bPendingEnableSplitscreen = false;
	}
}

void UArenaGameInstance::OnPostLoadMap()
{
	// Make sure we hide the loading screen when the level is done loading
	//UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

	//if (ShooterViewport != NULL)
	//{
	//	ShooterViewport->HideLoadingScreen();
	//}
}

void UArenaGameInstance::OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	/*if (WelcomeMenuUI.IsValid())
	{
		WelcomeMenuUI->LockControls(false);
	}*/

	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		if (UserId == *PendingInvite.UserId)
		{
			PendingInvite.bPrivilegesCheckedAndAllowed = true;
		}
	}
	else
	{
		DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
		GotoState(ArenaGameInstanceState::WelcomeScreen);
	}
}

void UArenaGameInstance::OnPostDemoPlay()
{
	GotoState(ArenaGameInstanceState::Playing);
}

void UArenaGameInstance::HandleDemoPlaybackFailure(EDemoPlayFailure::Type FailureType, const FString& ErrorString)
{
	ShowMessageThenGotoState(FText::Format(NSLOCTEXT("UShooterGameInstance", "DemoPlaybackFailedFmt", "Demo playback failed: {0}"), FText::FromString(ErrorString)), NSLOCTEXT("DialogButtons", "OKAY", "OK"), FText::GetEmpty(), ArenaGameInstanceState::MainMenu);
}

void UArenaGameInstance::StartGameInstance()
{
#if PLATFORM_PS4 == 0
	TCHAR Parm[4096] = TEXT("");

	const TCHAR* Cmd = FCommandLine::Get();

	// Catch the case where we want to override the map name on startup (used for connecting to other MP instances)
	if (FParse::Token(Cmd, Parm, ARRAY_COUNT(Parm), 0) && Parm[0] != '-')
	{
		// if we're 'overriding' with the default map anyway, don't set a bogus 'playing' state.
		if (!MainMenuMap.Contains(Parm))
		{
			FURL DefaultURL;
			DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);

			FURL URL(&DefaultURL, Parm, TRAVEL_Partial);

			if (URL.Valid)
			{
				UEngine* const Engine = GetEngine();

				FString Error;

				const EBrowseReturnVal::Type BrowseRet = Engine->Browse(*WorldContext, URL, Error);

				if (BrowseRet == EBrowseReturnVal::Success)
				{
					// Success, we loaded the map, go directly to playing state
					GotoState(ArenaGameInstanceState::Playing);
					return;
				}
				else if (BrowseRet == EBrowseReturnVal::Pending)
				{
					// Assume network connection
					LoadFrontEndMap(MainMenuMap);
					AddNetworkFailureHandlers();
					ShowLoadingScreen();
					GotoState(ArenaGameInstanceState::Playing);
					return;
				}
			}
		}
	}
#endif

	GotoInitialState();
}

FName UArenaGameInstance::GetInitialState()
{
#if ARENA_CONSOLE_UI	
	// Start in the welcome screen state on consoles
	return ArenaGameInstanceState::WelcomeScreen;
#else
	// On PC, go directly to the main menu
	return ArenaGameInstanceState::MainMenu;
#endif
}

void UArenaGameInstance::GotoInitialState()
{
	GotoState(GetInitialState());
}

void UArenaGameInstance::ShowMessageThenGotoState(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString, const FName& NewState, const bool OverrideExisting, TWeakObjectPtr< ULocalPlayer > PlayerOwner)
{
	UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Message: %s, NewState: %s"), *Message.ToString(), *NewState.ToString());

	const bool bAtWelcomeScreen = PendingState == ArenaGameInstanceState::WelcomeScreen || CurrentState == ArenaGameInstanceState::WelcomeScreen;

	// Never override the welcome screen
	if (bAtWelcomeScreen)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (at welcome screen)."));
		return;
	}

	const bool bAlreadyAtMessageMenu = PendingState == ArenaGameInstanceState::MessageMenu || CurrentState == ArenaGameInstanceState::MessageMenu;
	const bool bAlreadyAtDestState = PendingState == NewState || CurrentState == NewState;

	// If we are already going to the message menu, don't override unless asked to
	if (bAlreadyAtMessageMenu && PendingMessage.NextState == NewState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 1)."));
		return;
	}

	// If we are already going to the message menu, and the next dest is welcome screen, don't override
	if (bAlreadyAtMessageMenu && PendingMessage.NextState == ArenaGameInstanceState::WelcomeScreen)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 2)."));
		return;
	}

	// If we are already at the dest state, don't override unless asked
	if (bAlreadyAtDestState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 3)"));
		return;
	}

	PendingMessage.DisplayString = Message;
	PendingMessage.OKButtonString = OKButtonString;
	PendingMessage.CancelButtonString = CancelButtonString;
	PendingMessage.NextState = NewState;
	PendingMessage.PlayerOwner = PlayerOwner;

	if (CurrentState == ArenaGameInstanceState::MessageMenu)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Forcing new message"));
		EndMessageMenuState();
		BeginMessageMenuState();
	}
	else
	{
		GotoState(ArenaGameInstanceState::MessageMenu);
	}
}

void UArenaGameInstance::ShowLoadingScreen()
{
	// This can be confusing, so here is what is happening:
	//	For LoadMap, we use the IShooterGameLoadingScreenModule interface to show the load screen
	//  This is necessary since this is a blocking call, and our viewport loading screen won't get updated.
	//  We can't use IShooterGameLoadingScreenModule for seamless travel though
	//  In this case, we just add a widget to the viewport, and have it update on the main thread
	//  To simplify things, we just do both, and you can't tell, one will cover the other if they both show at the same time
	//IShooterGameLoadingScreenModule* const LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
	//if (LoadingScreenModule != nullptr)
	//{
	//	LoadingScreenModule->StartInGameLoadingScreen();
	//}

	//UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

	//if (ShooterViewport != NULL)
	//{
	//	ShooterViewport->ShowLoadingScreen();
	//}
}

bool UArenaGameInstance::LoadFrontEndMap(const FString& MapName)
{
	bool bSuccess = true;

	// if already loaded, do nothing
	UWorld* const World = GetWorld();
	if (World)
	{
		FString const CurrentMapName = *World->PersistentLevel->GetOutermost()->GetName();
		//if (MapName.Find(TEXT("Highrise")) != -1)
		if (CurrentMapName == MapName)
		{
			return bSuccess;
		}
	}

	FString Error;
	EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
	FURL URL(
		*FString::Printf(TEXT("%s"), *MapName)
		);

	if (URL.Valid && !HasAnyFlags(RF_ClassDefaultObject)) //CastChecked<UEngine>() will fail if using Default__ShooterGameInstance, so make sure that we're not default
	{
		BrowseRet = GetEngine()->Browse(*WorldContext, URL, Error);

		// Handle failure.
		if (BrowseRet != EBrowseReturnVal::Success)
		{
			UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *MapName, *Error));
			bSuccess = false;
		}
	}
	return bSuccess;
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

void UArenaGameInstance::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	AArenaPlayerController_Menu* const FirstPC = Cast<AArenaPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (FirstPC != nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.");
		if (ReasonString.IsEmpty() == false)
		{
			ReturnReason = FText::Format(NSLOCTEXT("NetworkErrors", "JoinSessionFailedReasonFmt", "Join Session failed. {0}"), FText::FromString(ReasonString));
		}

		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
	}
}

void UArenaGameInstance::ShowMessageThenGoMain(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString)
{
	ShowMessageThenGotoState(Message, OKButtonString, CancelButtonString, ArenaGameInstanceState::MainMenu);
}

void UArenaGameInstance::GotoState(FName NewState)
{
	UE_LOG(LogOnline, Log, TEXT("GotoState: NewState: %s"), *NewState.ToString());

	PendingState = NewState;
}

void UArenaGameInstance::MaybeChangeState()
{
	if ((PendingState != CurrentState) && (PendingState != ArenaGameInstanceState::None))
	{
		FName const OldState = CurrentState;

		// end current state
		EndCurrentState(PendingState);

		// begin new state
		BeginNewState(PendingState, OldState);

		// clear pending change
		PendingState = ArenaGameInstanceState::None;
	}
}

void UArenaGameInstance::EndCurrentState(FName NextState)
{
	// per-state custom ending code here
	if (CurrentState == ArenaGameInstanceState::PendingInvite)
	{
		EndPendingInviteState();
	}
	else if (CurrentState == ArenaGameInstanceState::WelcomeScreen)
	{
		EndWelcomeScreenState();
	}
	else if (CurrentState == ArenaGameInstanceState::MainMenu)
	{
		EndMainMenuState();
	}
	else if (CurrentState == ArenaGameInstanceState::MessageMenu)
	{
		EndMessageMenuState();
	}
	else if (CurrentState == ArenaGameInstanceState::Playing)
	{
		EndPlayingState();
	}

	CurrentState = ArenaGameInstanceState::None;
}

void UArenaGameInstance::BeginNewState(FName NewState, FName PrevState)
{
	// per-state custom starting code here

	if (NewState == ArenaGameInstanceState::PendingInvite)
	{
		BeginPendingInviteState();
	}
	else if (NewState == ArenaGameInstanceState::WelcomeScreen)
	{
		BeginWelcomeScreenState();
	}
	else if (NewState == ArenaGameInstanceState::MainMenu)
	{
		BeginMainMenuState();
	}
	else if (NewState == ArenaGameInstanceState::MessageMenu)
	{
		BeginMessageMenuState();
	}
	else if (NewState == ArenaGameInstanceState::Playing)
	{
		BeginPlayingState();
	}

	CurrentState = NewState;
}

void UArenaGameInstance::BeginPendingInviteState()
{
	if (LoadFrontEndMap(MainMenuMap))
	{
		StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UArenaGameInstance::OnUserCanPlayInvite), EUserPrivileges::CanPlayOnline, PendingInvite.UserId);
	}
	else
	{
		GotoState(ArenaGameInstanceState::WelcomeScreen);
	}
}

void UArenaGameInstance::EndPendingInviteState()
{
	// cleanup in case the state changed before the pending invite was handled.
	CleanupOnlinePrivilegeTask();
}

void UArenaGameInstance::BeginWelcomeScreenState()
{
	//this must come before split screen player removal so that the OSS sets all players to not using online features.
	SetIsOnline(false);

	// Remove any possible splitscren players
	RemoveSplitScreenPlayers();

	LoadFrontEndMap(WelcomeScreenMap);

	ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();
	LocalPlayer->SetCachedUniqueNetId(nullptr);
	/*check(!WelcomeMenuUI.IsValid());*/
	//WelcomeMenuUI = MakeShareable(new FShooterWelcomeMenu);
	//WelcomeMenuUI->Construct(this);
	//WelcomeMenuUI->AddToGameViewport();

	// Disallow splitscreen (we will allow while in the playing state)
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);
}

void UArenaGameInstance::EndWelcomeScreenState()
{
	/*if (WelcomeMenuUI.IsValid())
	{
		WelcomeMenuUI->RemoveFromGameViewport();
		WelcomeMenuUI = nullptr;
	}*/
}

void UArenaGameInstance::SetPresenceForLocalPlayers(const FVariantData& PresenceData)
{
	const auto Presence = Online::GetPresenceInterface();
	if (Presence.IsValid())
	{
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			const TSharedPtr<const FUniqueNetId> UserId = LocalPlayers[i]->GetPreferredUniqueNetId();

			if (UserId.IsValid())
			{
				FOnlineUserPresenceStatus PresenceStatus;
				PresenceStatus.Properties.Add(DefaultPresenceKey, PresenceData);

				Presence->SetPresence(*UserId, PresenceStatus);
			}
		}
	}
}

void UArenaGameInstance::BeginMainMenuState()
{
	// Make sure we're not showing the loadscreen
	/*UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

	if (ShooterViewport != NULL)
	{
		ShooterViewport->HideLoadingScreen();
	}*/

	SetIsOnline(false);

	// Disallow splitscreen
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);

	// Remove any possible splitscren players
	RemoveSplitScreenPlayers();

	// Set presence to menu state for the owning player
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	// load startup map
	LoadFrontEndMap(MainMenuMap);

	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();

	/*MainMenuUI = MakeShareable(new FShooterMainMenu());
	MainMenuUI->Construct(this, Player);
	MainMenuUI->AddMenuToGameViewport();*/

#if !ARENA_CONSOLE_UI
	// The cached unique net ID is usually set on the welcome screen, but there isn't
	// one on PC/Mac, so do it here.
	if (Player != nullptr)
	{
		Player->SetControllerId(0);
		Player->SetCachedUniqueNetId(Player->GetUniqueNetIdFromCachedControllerId());
	}
#endif

	RemoveNetworkFailureHandlers();
}

void UArenaGameInstance::EndMainMenuState()
{
	/*if (MainMenuUI.IsValid())
	{
		MainMenuUI->RemoveMenuFromGameViewport();
		MainMenuUI = nullptr;
	}*/
}

void UArenaGameInstance::BeginMessageMenuState()
{
	if (PendingMessage.DisplayString.IsEmpty())
	{
		UE_LOG(LogOnlineGame, Warning, TEXT("UArenaGameInstance::BeginMessageMenuState: Display string is empty"));
		GotoInitialState();
		return;
	}

	// Make sure we're not showing the loadscreen
	/*UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

	if (ShooterViewport != NULL)
	{
		ShooterViewport->HideLoadingScreen();
	}*/

	/*check(!MessageMenuUI.IsValid());
	MessageMenuUI = MakeShareable(new FShooterMessageMenu);
	MessageMenuUI->Construct(this, PendingMessage.PlayerOwner, PendingMessage.DisplayString, PendingMessage.OKButtonString, PendingMessage.CancelButtonString, PendingMessage.NextState);
*/
	PendingMessage.DisplayString = FText::GetEmpty();
}

void UArenaGameInstance::EndMessageMenuState()
{
	/*if (MessageMenuUI.IsValid())
	{
		MessageMenuUI->RemoveFromGameViewport();
		MessageMenuUI = nullptr;
	}*/
}

void UArenaGameInstance::BeginPlayingState()
{
	bPendingEnableSplitscreen = true;

	// Set presence for playing in a map
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("InGame"))));

	// Make sure viewport has focus
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void UArenaGameInstance::EndPlayingState()
{
	// Disallow splitscreen
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);

	// Clear the players' presence information
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	UWorld* const World = GetWorld();
	AArenaGameState* const GameState = World != NULL ? World->GetGameState<AArenaGameState>() : NULL;

	if (GameState)
	{
		// Send round end events for local players
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			auto ArenaPC = Cast<AArenaPlayerController>(LocalPlayers[i]->PlayerController);
			if (ArenaPC)
			{
				// Assuming you can't win if you quit early
				ArenaPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
			}
		}

		// Give the game state a chance to cleanup first
		GameState->RequestFinishAndExitToMainMenu();
	}
	else
	{
		// If there is no game state, make sure the session is in a good state
		CleanupSessionOnReturnToMenu();
	}
}

void UArenaGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("UArenaGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
			Sessions->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		}
	}

	// continue
	CleanupSessionOnReturnToMenu();
}

void UArenaGameInstance::CleanupSessionOnReturnToMenu()
{
	bool bPendingOnlineOp = false;

	// end online game and then destroy it
	IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

	if (Sessions.IsValid())
	{
		EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
		UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSessionName.ToString(), EOnlineSessionState::ToString(SessionState));

		if (EOnlineSessionState::InProgress == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->EndSession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSessionName.ToString());
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Starting == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
	}

	if (!bPendingOnlineOp)
	{
		GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
	}
}

void UArenaGameInstance::LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const
{
	AArenaPlayerState* const PlayerState = LocalPlayer && LocalPlayer->PlayerController ? Cast<AArenaPlayerState>(LocalPlayer->PlayerController->PlayerState) : nullptr;
	if (PlayerState)
	{
		PlayerState->SetQuitter(true);
	}
}

void UArenaGameInstance::RemoveNetworkFailureHandlers()
{
	// Remove the local session/travel failure bindings if they exist
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnTravelFailure().Remove(TravelLocalSessionFailureDelegateHandle);
	}
}

void UArenaGameInstance::AddNetworkFailureHandlers()
{
	// Add network/travel error handlers (if they are not already there)
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
	{
		TravelLocalSessionFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &UArenaGameInstance::TravelLocalSessionFailure);
	}
}

// starts playing a game as the host
bool UArenaGameInstance::HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL)
{
	if (!GetIsOnline())
	{
		//
		// Offline game, just go straight to map
		//

		ShowLoadingScreen();
		GotoState(ArenaGameInstanceState::Playing);

		// Travel to the specified match URL
		TravelURL = InTravelURL;
		GetWorld()->ServerTravel(TravelURL);
		return true;
	}

	//
	// Online game
	//

	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		// add callback delegate for completion
		OnCreatePresenceSessionCompleteDelegateHandle = GameSession->OnCreatePresenceSessionComplete().AddUObject(this, &UArenaGameInstance::OnCreatePresenceSessionComplete);

		TravelURL = InTravelURL;
		bool const bIsLanMatch = InTravelURL.Contains(TEXT("?bIsLanMatch"));

		//determine the map name from the travelURL
		const FString& MapNameSubStr = "/Game/Maps/";
		const FString& ChoppedMapName = TravelURL.RightChop(MapNameSubStr.Len());
		const FString& MapName = ChoppedMapName.LeftChop(ChoppedMapName.Len() - ChoppedMapName.Find("?game"));

		if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, GameType, MapName, bIsLanMatch, true, AArenaGameSession::DEFAULT_NUM_PLAYERS))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ArenaGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(ArenaGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

bool UArenaGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
	// needs to tear anything down based on current state?

	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &UArenaGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SessionIndexInSearchResults))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ArenaGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(ArenaGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

bool UArenaGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	// needs to tear anything down based on current state?
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &UArenaGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SearchResult))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == ArenaGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(ArenaGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

bool UArenaGameInstance::PlayDemo(ULocalPlayer* LocalPlayer, const FString& DemoName)
{
	ShowLoadingScreen();

	// Play the demo
	PlayReplay(DemoName);

	return true;
}

/** Callback which is intended to be called upon finding sessions */
void UArenaGameInstance::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	// unhook the delegate
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->OnJoinSessionComplete().Remove(OnJoinSessionCompleteDelegateHandle);
	}

	// Add the splitscreen player if one exists
	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UArenaGameInstance::OnRegisterJoiningLocalPlayerComplete));
		}
	}
	else
	{
		// We either failed or there is only a single local user
		FinishJoinSession(Result);
	}
}

void UArenaGameInstance::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	InternalTravelToSession(GameSessionName);
}

void UArenaGameInstance::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void UArenaGameInstance::InternalTravelToSession(const FName& SessionName)
{
	APlayerController * const PlayerController = GetFirstLocalPlayerController();

	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	// travel to session
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "OSSMissing", "OSS missing");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	FString URL;
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (!Sessions.IsValid() || !Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(FailReason, OKButton, FText::GetEmpty());
		UE_LOG(LogOnlineGame, Warning, TEXT("Failed to travel to session upon joining it"));
		return;
	}

	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

/** Callback which is intended to be called upon session creation */
void UArenaGameInstance::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
	AArenaGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->OnCreatePresenceSessionComplete().Remove(OnCreatePresenceSessionCompleteDelegateHandle);

		// Add the splitscreen player if one exists
		if (bWasSuccessful && LocalPlayers.Num() > 1)
		{
			auto Sessions = Online::GetSessionInterface();
			if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
			{
				Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
					FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UArenaGameInstance::OnRegisterLocalPlayerComplete));
			}
		}
		else
		{
			// We either failed or there is only a single local user
			FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
		}
	}
}

/** Initiates the session searching */
bool UArenaGameInstance::FindSessions(ULocalPlayer* PlayerOwner, bool bFindLAN)
{
	bool bResult = false;

	check(PlayerOwner != nullptr);
	if (PlayerOwner)
	{
		AArenaGameSession* const GameSession = GetGameSession();
		if (GameSession)
		{
			GameSession->OnFindSessionsComplete().RemoveAll(this);
			OnSearchSessionsCompleteDelegateHandle = GameSession->OnFindSessionsComplete().AddUObject(this, &UArenaGameInstance::OnSearchSessionsComplete);

			GameSession->FindSessions(PlayerOwner->GetPreferredUniqueNetId(), GameSessionName, bFindLAN, true);

			bResult = true;
		}
	}

	return bResult;
}

/** Callback which is intended to be called upon finding sessions */
void UArenaGameInstance::OnSearchSessionsComplete(bool bWasSuccessful)
{
	AArenaGameSession* const Session = GetGameSession();
	if (Session)
	{
		Session->OnFindSessionsComplete().Remove(OnSearchSessionsCompleteDelegateHandle);
	}
}

bool UArenaGameInstance::Tick(float DeltaSeconds)
{
	// Dedicated server doesn't need to worry about game state
	if (IsRunningDedicatedServer() == true)
	{
		return true;
	}

	MaybeChangeState();

//	UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
//
//	if (CurrentState != ArenaGameInstanceState::WelcomeScreen)
//	{
//		// If at any point we aren't licensed (but we are after welcome screen) bounce them back to the welcome screen
//		if (!bIsLicensed && CurrentState != ArenaGameInstanceState::None && !ShooterViewport->IsShowingDialog())
//		{
//			const FText ReturnReason = NSLOCTEXT("ProfileMessages", "NeedLicense", "The signed in users do not have a license for this game. Please purchase ShooterGame from the Xbox Marketplace or sign in a user with a valid license.");
//			const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
//
//			ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ArenaGameInstanceState::WelcomeScreen);
//		}
//
//		// Show controller disconnected dialog if any local players have an invalid controller
//		if (ShooterViewport != NULL &&
//			!ShooterViewport->IsShowingDialog())
//		{
//			for (int i = 0; i < LocalPlayers.Num(); ++i)
//			{
//				if (LocalPlayers[i] && LocalPlayers[i]->GetControllerId() == -1)
//				{
//					ShooterViewport->ShowDialog(
//						LocalPlayers[i],
//						EShooterDialogType::ControllerDisconnected,
//						FText::Format(NSLOCTEXT("ProfileMessages", "PlayerReconnectControllerFmt", "Player {0}, please reconnect your controller."), FText::AsNumber(i + 1)),
//#ifdef PLATFORM_PS4
//						NSLOCTEXT("DialogButtons", "PS4_CrossButtonContinue", "Cross Button - Continue"),
//#else
//						NSLOCTEXT("DialogButtons", "AButtonContinue", "A - Continue"),
//#endif
//						FText::GetEmpty(),
//						FOnClicked::CreateUObject(this, &UArenaGameInstance::OnControllerReconnectConfirm),
//						FOnClicked()
//						);
//				}
//			}
//		}
//	}

	// If we have a pending invite, and we are at the welcome screen, and the session is properly shut down, accept it
	if (PendingInvite.UserId.IsValid() && PendingInvite.bPrivilegesCheckedAndAllowed && CurrentState == ArenaGameInstanceState::PendingInvite)
	{
		IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
		IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);

			if (SessionState == EOnlineSessionState::NoSession)
			{
				ULocalPlayer * NewPlayerOwner = GetFirstGamePlayer();

				if (NewPlayerOwner != nullptr)
				{
					NewPlayerOwner->SetControllerId(PendingInvite.ControllerId);
					NewPlayerOwner->SetCachedUniqueNetId(PendingInvite.UserId);
					SetIsOnline(true);
					JoinSession(NewPlayerOwner, PendingInvite.InviteResult);
				}

				PendingInvite.UserId.Reset();
			}
		}
	}

	return true;
}

bool UArenaGameInstance::HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld)
{
	bool const bOpenSuccessful = Super::HandleOpenCommand(Cmd, Ar, InWorld);
	if (bOpenSuccessful)
	{
		GotoState(ArenaGameInstanceState::Playing);
	}

	return bOpenSuccessful;
}

void UArenaGameInstance::HandleSignInChangeMessaging()
{
	// Master user signed out, go to initial state (if we aren't there already)
	if (CurrentState != GetInitialState())
	{
#if ARENA_CONSOLE_UI
		// Display message on consoles
		const FText ReturnReason = NSLOCTEXT("ProfileMessages", "SignInChange", "Sign in status change occurred.");
		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

		ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), GetInitialState());
#else								
		GotoInitialState();
#endif
	}
}

void UArenaGameInstance::HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId)
{
	const bool bDowngraded = (LoginStatus == ELoginStatus::NotLoggedIn && !GetIsOnline()) || (LoginStatus != ELoginStatus::LoggedIn && GetIsOnline());

	UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: bDownGraded: %i"), (int)bDowngraded);

	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	bIsLicensed = GenericApplication->ApplicationLicenseValid();

	// Find the local player associated with this unique net id
	ULocalPlayer * LocalPlayer = FindLocalPlayerFromUniqueNetId(UserId);

	// If this user is signed out, but was previously signed in, punt to welcome (or remove splitscreen if that makes sense)
	if (LocalPlayer != NULL)
	{
		if (bDowngraded)
		{
			UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: Player logged out: %s"), *UserId.ToString());

			LabelPlayerAsQuitter(LocalPlayer);

			// Check to see if this was the master, or if this was a split-screen player on the client
			if (LocalPlayer == GetFirstGamePlayer() || GetIsOnline())
			{
				HandleSignInChangeMessaging();
			}
			else
			{
				// Remove local split-screen players from the list
				RemoveExistingLocalPlayer(LocalPlayer);
			}
		}
	}
}

void UArenaGameInstance::HandleAppWillDeactivate()
{
	if (CurrentState == ArenaGameInstanceState::Playing)
	{
		// Just have the first player controller pause the game.
		UWorld* const GameWorld = GetWorld();
		if (GameWorld)
		{
			// protect against a second pause menu loading on top of an existing one if someone presses the Jewel / PS buttons.
			bool bNeedsPause = true;
			for (FConstControllerIterator It = GameWorld->GetControllerIterator(); It; ++It)
			{
				AArenaPlayerController* Controller = Cast<AArenaPlayerController>(*It);
				if (Controller && (Controller->IsPaused()))// || Controller->IsGameMenuVisible()))
				{
					bNeedsPause = false;
					break;
				}
			}

			if (bNeedsPause)
			{
				AArenaPlayerController* const Controller = Cast<AArenaPlayerController>(GameWorld->GetFirstPlayerController());
				if (Controller)
				{
					//Controller->ShowInGameMenu();
				}
			}
		}
	}
}

void UArenaGameInstance::HandleAppSuspend()
{
	// Players will lose connection on resume. However it is possible the game will exit before we get a resume, so we must kick off round end events here.
	UE_LOG(LogOnline, Warning, TEXT("UArenaGameInstance::HandleAppSuspend"));
	UWorld* const World = GetWorld();
	AArenaGameState* const GameState = World != NULL ? World->GetGameState<AArenaGameState>() : NULL;

	if (CurrentState != ArenaGameInstanceState::None && CurrentState != GetInitialState())
	{
		UE_LOG(LogOnline, Warning, TEXT("UArenaGameInstance::HandleAppSuspend: Sending round end event for players"));

		// Send round end events for local players
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			auto ArenaPC = Cast<AArenaPlayerController>(LocalPlayers[i]->PlayerController);
			if (ArenaPC)
			{
				// Assuming you can't win if you quit early
				ArenaPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
			}
		}
	}
}

void UArenaGameInstance::HandleAppResume()
{
	UE_LOG(LogOnline, Log, TEXT("UArenaGameInstance::HandleAppResume"));

	if (CurrentState != ArenaGameInstanceState::None && CurrentState != GetInitialState())
	{
		UE_LOG(LogOnline, Warning, TEXT("UArenaGameInstance::HandleAppResume: Attempting to sign out players"));

		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			if (LocalPlayers[i]->GetCachedUniqueNetId().IsValid() && !IsLocalPlayerOnline(LocalPlayers[i]))
			{
				UE_LOG(LogOnline, Log, TEXT("UArenaGameInstance::HandleAppResume: Signed out during resume."));
				HandleSignInChangeMessaging();
				break;
			}
		}
	}
}

void UArenaGameInstance::HandleAppLicenseUpdate()
{
	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	bIsLicensed = GenericApplication->ApplicationLicenseValid();
}

void UArenaGameInstance::HandleSafeFrameChanged()
{
	UCanvas::UpdateAllCanvasSafeZoneData();
}

void UArenaGameInstance::RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	check(ExistingPlayer);
	if (ExistingPlayer->PlayerController != NULL)
	{
		// Kill the player
		AArenaCharacter* MyPawn = Cast<AArenaCharacter>(ExistingPlayer->PlayerController->GetPawn());
		if (MyPawn)
		{
			MyPawn->KilledBy(NULL);
		}
	}

	// Remove local split-screen players from the list
	RemoveLocalPlayer(ExistingPlayer);
}

void UArenaGameInstance::RemoveSplitScreenPlayers()
{
	// if we had been split screen, toss the extra players now
	// remove every player, back to front, except the first one
	while (LocalPlayers.Num() > 1)
	{
		ULocalPlayer* const PlayerToRemove = LocalPlayers.Last();
		RemoveExistingLocalPlayer(PlayerToRemove);
	}
}

FReply UArenaGameInstance::OnPairingUsePreviousProfile()
{
	// Do nothing (except hide the message) if they want to continue using previous profile
	/*UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

	if (ShooterViewport != nullptr)
	{
		ShooterViewport->HideDialog();
	}*/

	return FReply::Handled();
}

FReply UArenaGameInstance::OnPairingUseNewProfile()
{
	HandleSignInChangeMessaging();
	return FReply::Handled();
}

void UArenaGameInstance::HandleControllerPairingChanged(int GameUserIndex, const FUniqueNetId& PreviousUser, const FUniqueNetId& NewUser)
{
	UE_LOG(LogOnlineGame, Log, TEXT("UArenaGameInstance::HandleControllerPairingChanged GameUserIndex %d PreviousUser '%s' NewUser '%s'"),
		GameUserIndex, *PreviousUser.ToString(), *NewUser.ToString());

	if (CurrentState == ArenaGameInstanceState::WelcomeScreen)
	{
		// Don't care about pairing changes at welcome screen
		return;
	}

#if ARENA_CONSOLE_UI && PLATFORM_XBOXONE
	if (IgnorePairingChangeForControllerId != -1 && GameUserIndex == IgnorePairingChangeForControllerId)
	{
		// We were told to ignore
		IgnorePairingChangeForControllerId = -1;	// Reset now so there there is no chance this remains in a bad state
		return;
	}

	if (PreviousUser.IsValid() && !NewUser.IsValid())
	{
		// Treat this as a disconnect or signout, which is handled somewhere else
		return;
	}

	if (!PreviousUser.IsValid() && NewUser.IsValid())
	{
		// Treat this as a signin
		ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

		if (ControlledLocalPlayer != NULL && !ControlledLocalPlayer->GetCachedUniqueNetId().IsValid())
		{
			// If a player that previously selected "continue without saving" signs into this controller, move them back to welcome screen
			HandleSignInChangeMessaging();
		}

		return;
	}

	// Find the local player currently being controlled by this controller
	ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

	// See if the newly assigned profile is in our local player list
	ULocalPlayer * NewLocalPlayer = FindLocalPlayerFromUniqueNetId(NewUser);

	// If the local player being controlled is not the target of the pairing change, then give them a chance 
	// to continue controlling the old player with this controller
	if (ControlledLocalPlayer != nullptr && ControlledLocalPlayer != NewLocalPlayer)
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

		if (ShooterViewport != nullptr)
		{
			ShooterViewport->ShowDialog(
				nullptr,
				EShooterDialogType::Generic,
				NSLOCTEXT("ProfileMessages", "PairingChanged", "Your controller has been paired to another profile, would you like to switch to this new profile now? Selecting YES will sign out of the previous profile."),
				NSLOCTEXT("DialogButtons", "YES", "A - YES"),
				NSLOCTEXT("DialogButtons", "NO", "B - NO"),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnPairingUseNewProfile),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnPairingUsePreviousProfile)
				);
		}
	}
#endif
}

void UArenaGameInstance::HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex)
{
	UE_LOG(LogOnlineGame, Log, TEXT("UArenaGameInstance::HandleControllerConnectionChange bIsConnection %d GameUserIndex %d"),
		bIsConnection, GameUserIndex);

	if (!bIsConnection)
	{
		// Controller was disconnected

		// Find the local player associated with this user index
		ULocalPlayer * LocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

		if (LocalPlayer == NULL)
		{
			return;		// We don't care about players we aren't tracking
		}

		// Invalidate this local player's controller id.
		LocalPlayer->SetControllerId(-1);
	}
}

FReply UArenaGameInstance::OnControllerReconnectConfirm()
{
	/*UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
	if (ShooterViewport)
	{
		ShooterViewport->HideDialog();
	}*/

	return FReply::Handled();
}

TSharedPtr<const FUniqueNetId > UArenaGameInstance::GetUniqueNetIdFromControllerId(const int ControllerId)
{
	IOnlineIdentityPtr OnlineIdentityInt = Online::GetIdentityInterface();

	if (OnlineIdentityInt.IsValid())
	{
		TSharedPtr<const FUniqueNetId> UniqueId = OnlineIdentityInt->GetUniquePlayerId(ControllerId);

		if (UniqueId.IsValid())
		{
			return UniqueId;
		}
	}

	return nullptr;
}

void UArenaGameInstance::SetIsOnline(bool bInIsOnline)
{
	bIsOnline = bInIsOnline;
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			ULocalPlayer* LocalPlayer = LocalPlayers[i];

			TSharedPtr<const FUniqueNetId> PlayerId = LocalPlayer->GetPreferredUniqueNetId();
			if (PlayerId.IsValid())
			{
				OnlineSub->SetUsingMultiplayerFeatures(*PlayerId, bIsOnline);
			}
		}
	}
}

void UArenaGameInstance::TravelToSession(const FName& SessionName)
{
	// Added to handle failures when joining using quickmatch (handles issue of joining a game that just ended, i.e. during game ending timer)
	AddNetworkFailureHandlers();
	ShowLoadingScreen();
	GotoState(ArenaGameInstanceState::Playing);
	InternalTravelToSession(SessionName);
}

void UArenaGameInstance::SetIgnorePairingChangeForControllerId(const int32 ControllerId)
{
	IgnorePairingChangeForControllerId = ControllerId;
}

bool UArenaGameInstance::IsLocalPlayerOnline(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer == NULL)
	{
		return false;
	}
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			auto UniqueId = LocalPlayer->GetCachedUniqueNetId();
			if (UniqueId.IsValid())
			{
				const auto LoginStatus = IdentityInterface->GetLoginStatus(*UniqueId);
				if (LoginStatus == ELoginStatus::LoggedIn)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool UArenaGameInstance::ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer)
{
	// Get the viewport
	//UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());

#if PLATFORM_XBOXONE
	if (CurrentConnectionStatus != EOnlineServerConnectionStatus::Connected)
	{
		// Don't let them play online if they aren't connected to Xbox LIVE
		if (ShooterViewport != NULL)
		{
			const FText Msg = NSLOCTEXT("NetworkFailures", "ServiceDisconnected", "You must be connected to the Xbox LIVE service to play online.");
			const FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");

			ShooterViewport->ShowDialog(
				NULL,
				EShooterDialogType::Generic,
				Msg,
				OKButtonString,
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}

		return false;
	}
#endif

	if (!IsLocalPlayerOnline(LocalPlayer))
	{
		// Don't let them play online if they aren't online
		/*if (ShooterViewport != NULL)
		{
			const FText Msg = NSLOCTEXT("NetworkFailures", "MustBeSignedIn", "You must be signed in to play online");
			const FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");

			ShooterViewport->ShowDialog(
				NULL,
				EShooterDialogType::Generic,
				Msg,
				OKButtonString,
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}*/

		return false;
	}

	return true;
}

FReply UArenaGameInstance::OnConfirmGeneric()
{
	/*UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
	if (ShooterViewport)
	{
		ShooterViewport->HideDialog();
	}*/

	return FReply::Handled();
}

void UArenaGameInstance::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< FUniqueNetId > UserId)
{
	/*WaitMessageWidget = SNew(SShooterWaitDialog)
		.MessageText(NSLOCTEXT("NetworkStatus", "CheckingPrivilegesWithServer", "Checking privileges with server.  Please wait..."));*/

	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		//GVC->AddViewportWidgetContent(WaitMessageWidget.ToSharedRef());
	}

	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && UserId.IsValid())
	{
		Identity->GetUserPrivilege(*UserId, Privilege, Delegate);
	}
	else
	{
		// Can only get away with faking the UniqueNetId here because the delegates don't use it
		Delegate.ExecuteIfBound(FUniqueNetIdString(), Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
	}
}

void UArenaGameInstance::CleanupOnlinePrivilegeTask()
{
	if (GEngine && GEngine->GameViewport)// && WaitMessageWidget.IsValid())
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		//GVC->RemoveViewportWidgetContent(WaitMessageWidget.ToSharedRef());
	}
}

void UArenaGameInstance::DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	// Show warning that the user cannot play due to age restrictions
	//UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
	TWeakObjectPtr<ULocalPlayer> OwningPlayer;
	if (GEngine)
	{
		for (auto It = GEngine->GetLocalPlayerIterator(GetWorld()); It; ++It)
		{
			TSharedPtr<const FUniqueNetId> OtherId = (*It)->GetPreferredUniqueNetId();
			if (OtherId.IsValid())
			{
				if (UserId == (*OtherId))
				{
					OwningPlayer = *It;
				}
			}
		}
	}

	/*if (ShooterViewport != NULL && OwningPlayer.IsValid())
	{
		if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure) != 0)
		{
			IOnlineExternalUIPtr ExternalUI = Online::GetExternalUIInterface();
			if (ExternalUI.IsValid())
			{
				ExternalUI->ShowAccountUpgradeUI(UserId);
			}
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate) != 0)
		{
			ShooterViewport->ShowDialog(
				OwningPlayer.Get(),
				EShooterDialogType::Generic,
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredSystemUpdate", "A required system update is available.  Please upgrade to access online features."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"),
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) != 0)
		{
			ShooterViewport->ShowDialog(
				OwningPlayer.Get(),
				EShooterDialogType::Generic,
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredPatchAvailable", "A required game patch is available.  Please upgrade to access online features."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"),
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure) != 0)
		{
			ShooterViewport->ShowDialog(
				OwningPlayer.Get(),
				EShooterDialogType::Generic,
				NSLOCTEXT("OnlinePrivilegeResult", "AgeRestrictionFailure", "Cannot play due to age restrictions!"),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"),
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) != 0)
		{
			ShooterViewport->ShowDialog(
				OwningPlayer.Get(),
				EShooterDialogType::Generic,
				NSLOCTEXT("OnlinePrivilegeResult", "UserNotFound", "Cannot play due invalid user!"),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"),
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::GenericFailure) != 0)
		{
			ShooterViewport->ShowDialog(
				OwningPlayer.Get(),
				EShooterDialogType::Generic,
				NSLOCTEXT("OnlinePrivilegeResult", "GenericFailure", "Cannot play online.  Check your network connection."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"),
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric),
				FOnClicked::CreateUObject(this, &UArenaGameInstance::OnConfirmGeneric)
				);
		}
	}*/
}

void UArenaGameInstance::OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishSessionCreation(Result);
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
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
	}
}

void UArenaGameInstance::BeginHostingQuickMatch()
{
	ShowLoadingScreen();
	GotoState(ArenaGameInstanceState::Playing);

	// Travel to the specified match URL
	GetWorld()->ServerTravel(TEXT("/Game/Maps/StagingArea?game=FFA?listen"));
}
