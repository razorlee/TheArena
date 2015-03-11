#include "TheArena.h"
#include "OnlineAchievementsInterface.h"

#define  ACH_FRAG_SOMEONE	TEXT("ACH_FRAG_SOMEONE")
#define  ACH_SOME_KILLS		TEXT("ACH_SOME_KILLS")
#define  ACH_LOTS_KILLS		TEXT("ACH_LOTS_KILLS")
#define  ACH_FINISH_MATCH	TEXT("ACH_FINISH_MATCH")
#define  ACH_LOTS_MATCHES	TEXT("ACH_LOTS_MATCHES")
#define  ACH_FIRST_WIN		TEXT("ACH_FIRST_WIN")
#define  ACH_LOTS_WIN		TEXT("ACH_LOTS_WIN")
#define  ACH_MANY_WIN		TEXT("ACH_MANY_WIN")
#define  ACH_SHOOT_BULLETS	TEXT("ACH_SHOOT_BULLETS")
#define  ACH_SHOOT_ROCKETS	TEXT("ACH_SHOOT_ROCKETS")
#define  ACH_GOOD_SCORE		TEXT("ACH_GOOD_SCORE")
#define  ACH_GREAT_SCORE	TEXT("ACH_GREAT_SCORE")
#define  ACH_PLAY_SANCTUARY	TEXT("ACH_PLAY_SANCTUARY")
#define  ACH_PLAY_HIGHRISE	TEXT("ACH_PLAY_HIGHRISE")

static const int32 SomeKillsCount = 10;
static const int32 LotsKillsCount = 20;
static const int32 LotsMatchesCount = 5;
static const int32 LotsWinsCount = 3;
static const int32 ManyWinsCount = 5;
static const int32 LotsBulletsCount = 100;
static const int32 LotsRocketsCount = 10;
static const int32 GoodScoreCount = 10;
static const int32 GreatScoreCount = 15;


AArenaPlayerController::AArenaPlayerController(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	PlayerCameraManagerClass = AArenaPlayerCameraManager::StaticClass();
	//CheatClass = UArenaCheatManager::StaticClass();
	bAllowGameActions = true;
	LastDeathLocation = FVector::ZeroVector;
	OpenMenu = false;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnStartSessionCompleteEndItDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &AArenaPlayerController::OnStartSessionCompleteEndIt);
		OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &AArenaPlayerController::OnEndSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &AArenaPlayerController::OnDestroySessionComplete);
	}
	ServerSayString = TEXT("Say");
}

void AArenaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AArenaPlayerController::OnToggleInGameMenu);

	// voice chat
	InputComponent->BindAction("PushToTalk", IE_Pressed, this, &APlayerController::StartTalking);
	InputComponent->BindAction("PushToTalk", IE_Released, this, &APlayerController::StopTalking);
}

void AArenaPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//FArenaStyle::Initialize();
}

void AArenaPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AArenaPlayerController::UnFreeze()
{
	ServerRestartPlayer();
}

void AArenaPlayerController::FailedToSpawnPawn()
{
	if (StateName == NAME_Inactive)
	{
		BeginInactiveState();
	}
	Super::FailedToSpawnPawn();
}

void AArenaPlayerController::PawnPendingDestroy(APawn* P)
{
	LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	//FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(P);
}

void AArenaPlayerController::GameHasEnded(class AActor* EndGameFocus, bool bIsWinner)
{
	// write stats
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
		if (ArenaPlayerState)
		{
			// update local saved profile
			/*UArenaPersistentUser* const PersistentUser = GetPersistentUser();
			if (PersistentUser)
			{
				PersistentUser->AddMatchResult(ArenaPlayerState->GetKills(), ArenaPlayerState->GetDeaths(), ArenaPlayerState->GetNumBulletsFired(), ArenaPlayerState->GetNumRocketsFired(), bIsWinner);
				PersistentUser->SaveIfDirty();
			}*/

			// update leaderboards
			IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
			if (OnlineSub)
			{
				IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
				if (Identity.IsValid())
				{
					TSharedPtr<FUniqueNetId> UserId = Identity->GetUniquePlayerId(LocalPlayer->GetControllerId());
					if (UserId.IsValid())
					{
						IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
						if (Leaderboards.IsValid())
						{
							/*FArenaAllTimeMatchResultsWrite WriteObject;

							WriteObject.SetIntStat(LEADERBOARD_STAT_SCORE, ArenaPlayerState->GetKills());
							WriteObject.SetIntStat(LEADERBOARD_STAT_KILLS, ArenaPlayerState->GetKills());
							WriteObject.SetIntStat(LEADERBOARD_STAT_DEATHS, ArenaPlayerState->GetDeaths());
							WriteObject.SetIntStat(LEADERBOARD_STAT_MATCHESPLAYED, 1);

							// the call will copy the user id and write object to its own memory
							Leaderboards->WriteLeaderboards(ArenaPlayerState->SessionName, *UserId, WriteObject);*/
						}
					}
				}
			}
		}
	}

	Super::GameHasEnded(EndGameFocus, bIsWinner);
}

/*bool AArenaPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AArenaPlayerController::ServerCheat_Implementation(const FString& Msg)
{
	if (CheatManager)
	{
		ClientMessage(ConsoleCommand(Msg));
	}
}*/

/*void AArenaPlayerController::SimulateInputKey(FKey Key, bool bPressed)
{
	InputKey(Key, bPressed ? IE_Pressed : IE_Released, 1, false);
}*/

void AArenaPlayerController::OnKill()
{
	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	if (Events.IsValid() && Identity.IsValid())
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
		if (LocalPlayer)
		{
			int32 UserIndex = LocalPlayer->GetControllerId();
			TSharedPtr<FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
			if (UniqueID.IsValid())
			{
				ACharacter* Pawn = GetCharacter();
				// If player is dead, use location stored during pawn cleanup.
				FVector Location = LastDeathLocation;
				if (Pawn)
				{
					Pawn->GetActorLocation();
				}

				FOnlineEventParms Params;

				Params.Add(TEXT("SectionId"), FVariantData((int32)1));
				Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1));
				Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0));

				Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0));
				Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)0));
				Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0));
				Params.Add(TEXT("KillTypeId"), FVariantData((int32)0));
				Params.Add(TEXT("LocationX"), FVariantData(Location.X));
				Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
				Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));
				Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0));

				Events->TriggerEvent(*UniqueID, TEXT("KillOponent"), Params);
			}
		}
	}
}

void AArenaPlayerController::OnToggleInGameMenu()
{
	if (OpenMenu == true)
	{
		SetMenu(false);
		bAllowGameActions = true;
	}
	else
	{
		SetMenu(true);
		bAllowGameActions = false;
	}
	if (OpenFriendsList == true)
	{
		SetFriendsList(false);
	}
}

void AArenaPlayerController::SetInfiniteAmmo(bool bEnable)
{
	bInfiniteAmmo = bEnable;
}

void AArenaPlayerController::SetInfiniteClip(bool bEnable)
{
	bInfiniteClip = bEnable;
}

void AArenaPlayerController::SetHealthRegen(bool bEnable)
{
	bHealthRegen = bEnable;
}

void AArenaPlayerController::SetGodMode(bool bEnable)
{
	bGodMode = bEnable;
}

void AArenaPlayerController::SetMenu(bool bEnable)
{
	this->OpenMenu = bEnable;
}

void AArenaPlayerController::SetFriendsList(bool bEnable)
{
	this->OpenFriendsList = bEnable;
}

void AArenaPlayerController::SetHUD(bool bEnable)
{
	this->OpenHUD = bEnable;
}

void AArenaPlayerController::SetAllowGameActions(bool bEnable)
{
	bAllowGameActions = bEnable;
}

void AArenaPlayerController::ClientGameStarted_Implementation()
{
	/*AArenaHUD* ArenaHUD = GetArenaHUD();
	if (ArenaHUD)
	{
		ArenaHUD->SetMatchState(EArenaMatchState::Playing);
	}

	QueryAchievements();*/
}

/** Starts the online game using the session name in the PlayerState */
void AArenaPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (ArenaPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *ArenaPlayerState->SessionName.ToString());
				Sessions->StartSession(ArenaPlayerState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(FTimerDelegate::CreateUObject(this, &AArenaPlayerController::ClientStartOnlineGame_Implementation), 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void AArenaPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (ArenaPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *ArenaPlayerState->SessionName.ToString());
				Sessions->EndSession(ArenaPlayerState->SessionName);
			}
		}
	}
}

void AArenaPlayerController::ClientReturnToMainMenu_Implementation(const FString& ReturnReason)
{
	CleanupSessionOnReturnToMenu();
}

/** Ends and/or destroys game session */
void AArenaPlayerController::CleanupSessionOnReturnToMenu()
{
	bool bPendingOnlineOp = false;

	// end online game and then destroy it
	AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (ArenaPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				EOnlineSessionState::Type SessionState = Sessions->GetSessionState(ArenaPlayerState->SessionName);
				UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *ArenaPlayerState->SessionName.ToString(), EOnlineSessionState::ToString(SessionState));

				if (EOnlineSessionState::InProgress == SessionState)
				{
					UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *ArenaPlayerState->SessionName.ToString());
					Sessions->AddOnEndSessionCompleteDelegate(OnEndSessionCompleteDelegate);
					Sessions->EndSession(ArenaPlayerState->SessionName);
					bPendingOnlineOp = true;
				}
				else if (EOnlineSessionState::Ending == SessionState)
				{
					UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *ArenaPlayerState->SessionName.ToString());
					Sessions->AddOnEndSessionCompleteDelegate(OnEndSessionCompleteDelegate);
					bPendingOnlineOp = true;
				}
				else if (EOnlineSessionState::Ended == SessionState ||
					EOnlineSessionState::Pending == SessionState)
				{
					UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *ArenaPlayerState->SessionName.ToString());
					Sessions->AddOnDestroySessionCompleteDelegate(OnDestroySessionCompleteDelegate);
					Sessions->DestroySession(ArenaPlayerState->SessionName);
					bPendingOnlineOp = true;
				}
				else if (EOnlineSessionState::Starting == SessionState)
				{
					UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *ArenaPlayerState->SessionName.ToString());
					Sessions->AddOnStartSessionCompleteDelegate(OnStartSessionCompleteEndItDelegate);
					bPendingOnlineOp = true;
				}
			}
		}
	}

	if (!bPendingOnlineOp)
	{
		GEngine->HandleDisconnect(GetWorld(), GetWorld()->GetNetDriver());
	}
}

void AArenaPlayerController::OnStartSessionCompleteEndIt(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("OnStartSessionCompleteEndIt: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate(OnStartSessionCompleteEndItDelegate);
		}
	}

	// continue
	CleanupSessionOnReturnToMenu();
}

/**
* Delegate fired when ending an online session has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaPlayerController::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnEndSessionCompleteDelegate(OnEndSessionCompleteDelegate);
		}
	}

	// continue
	CleanupSessionOnReturnToMenu();
}

/**
* Delegate fired when destroying an online session has completed
*
* @param SessionName the name of the session this callback is for
* @param bWasSuccessful true if the async action completed without error, false if there was an error
*/
void AArenaPlayerController::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("OnDestroySessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnDestroySessionCompleteDelegate(OnDestroySessionCompleteDelegate);
		}
	}

	// continue
	if (bWasSuccessful)
	{
		CleanupSessionOnReturnToMenu();
	}
}

void AArenaPlayerController::ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner)
{
	Super::ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

	// Allow only looking around
	SetIgnoreMoveInput(true);
	bAllowGameActions = false;

	// Make sure that we still have valid view target
	SetViewTarget(GetPawn());
}

void AArenaPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	// If we have a pawn we need to determine if we should show/hide the weapon
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(GetPawn());
	AArenaRangedWeapon* MyWeapon = MyPawn ? MyPawn->GetWeapon() : NULL;
	if (MyWeapon)
	{
		if (bInCinematicMode && bHidePlayer)
		{
			MyWeapon->SetActorHiddenInGame(true);
		}
		else if (!bCinematicMode)
		{
			MyWeapon->SetActorHiddenInGame(false);
		}
	}
}

void AArenaPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	/*UArenaPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}*/
}

void AArenaPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AArenaPlayerController, bInfiniteAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AArenaPlayerController, bInfiniteClip, COND_OwnerOnly);
}

void AArenaPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}

bool AArenaPlayerController::ServerSuicide_Validate()
{
	return true;
}

void AArenaPlayerController::ServerSuicide_Implementation()
{
	if ((GetPawn() != NULL) && ((GetWorld()->TimeSeconds - GetPawn()->CreationTime > 1) || (GetNetMode() == NM_Standalone)))
	{
		AArenaCharacter* MyPawn = Cast<AArenaCharacter>(GetPawn());
		if (MyPawn)
		{
			MyPawn->Suicide();
		}
	}
}

bool AArenaPlayerController::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

bool AArenaPlayerController::HasInfiniteClip() const
{
	return bInfiniteClip;
}

bool AArenaPlayerController::HasHealthRegen() const
{
	return bHealthRegen;
}

bool AArenaPlayerController::HasGodMode() const
{
	return bGodMode;
}

bool AArenaPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

bool AArenaPlayerController::IsMenuOpen() const
{
	return this->OpenMenu;
}

bool AArenaPlayerController::IsFriendsListOpen() const
{
	return this->OpenFriendsList;
}

bool AArenaPlayerController::IsHUDOpen() const
{
	return this->OpenHUD;
}

bool AArenaPlayerController::GetAllowGameActions() const
{
	return bAllowGameActions;
}


