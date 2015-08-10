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
	bGameEndedFrame = false;
	LastDeathLocation = FVector::ZeroVector;
	OpenMenu = false;

	ServerSayString = TEXT("Say");
	ArenaFriendUpdateTimer = 0.0f;
	bHasSentStartEvents = false;
}

void AArenaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &AArenaPlayerController::OnToggleInGameMenu);
	InputComponent->BindAction("Inventory", IE_Pressed, this, &AArenaPlayerController::OnToggleInventory);

	// voice chat
	InputComponent->BindAction("PushToTalk", IE_Pressed, this, &APlayerController::StartTalking);
	InputComponent->BindAction("PushToTalk", IE_Released, this, &APlayerController::StopTalking);
}

void AArenaPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ArenaFriendUpdateTimer = 0;
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
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(P);

	ClientSetSpectatorCamera(CameraLocation, CameraRotation);
}

void AArenaPlayerController::GameHasEnded(class AActor* EndGameFocus, bool bIsWinner)
{
	//UpdateSaveFileOnGameEnd(bIsWinner);
	//UpdateAchievementsOnGameEnd();
	//UpdateLeaderboardsOnGameEnd();

	Super::GameHasEnded(EndGameFocus, bIsWinner);
}

void AArenaPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

bool AArenaPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 600.0f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;

		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

void AArenaPlayerController::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
	SetMatchOver(true);
	IsWinner = bIsWinner;

	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
			AArenaPlayerState* ArenaPlayerState = Cast<AArenaPlayerState>(PlayerState);
			int32 PlayerScore = ArenaPlayerState ? ArenaPlayerState->GetScore() : 0;

			// Fire session end event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("PlayerScore"), FVariantData((int32)PlayerScore));
			Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
			Params.Add(TEXT("MapName"), FVariantData(MapName));
			Params.Add(TEXT("MapNameString"), FVariantData(MapName)); // @todo workaround for a bug in backend service, remove when fixed

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

			// Online matches require the MultiplayerRoundEnd event as well
			UArenaGameInstance* AGI = GetWorld() != NULL ? Cast<UArenaGameInstance>(GetWorld()->GetGameInstance()) : NULL;
			if (AGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				AArenaGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<AArenaGameState>() : NULL;
				if (ensure(MyGameState != nullptr))
				{
					MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
					MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
					MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

					Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
				}
			}
		}

		bHasSentStartEvents = false;
	}
}

void AArenaPlayerController::SimulateInputKey(FKey Key, bool bPressed)
{
	InputKey(Key, bPressed ? IE_Pressed : IE_Released, 1, false);
}

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
				ACharacter* Pawn = Cast<AArenaCharacter>(GetCharacter());
				// If player is dead, use location stored during pawn cleanup.
				FVector Location = Pawn ? Pawn->GetActorLocation() : LastDeathLocation;

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

///////////////////////////////////////////////// INPUT /////////////////////////////////////////////////

void AArenaPlayerController::OnToggleInGameMenu()
{
	if (OpenSettings == true)
	{
		SetSettings(false);
	}
	if (OpenFriendsList == true)
	{
		SetFriendsList(false);
	}
	if (OpenMenu == true)
	{
		SetMenu(false);
		bAllowGameActions = true;

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		FInputModeGameOnly Input;
		SetInputMode(Input);
	}
	else
	{
		SetMenu(true);
		bAllowGameActions = false;

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
	}
}

void AArenaPlayerController::OnToggleInventory()
{
	if (OpenFriendsList == false && OpenMenu == false)
	{
		if (IsNearbyInventory())
		{
			if (IsInventoryOpen())
			{
				SetInventory(false);
				bShowMouseCursor = false;
				bEnableClickEvents = false;
				bEnableMouseOverEvents = false;
				return;
			}
			else
			{
				SetInventory(true);
				bShowMouseCursor = true;
				bEnableClickEvents = true;
				bEnableMouseOverEvents = true;
				return;
			}
		}
		SetInventory(false);
		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		return;
	}
	SetInventory(false);
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	return;
}

////////////////////////////////////////// Getters and Setters //////////////////////////////////////////

FText AArenaPlayerController::GetInteractiveMessage()
{
	return InteractiveMessage;
}
void AArenaPlayerController::SetInteractiveMessage(FText Message)
{
	InteractiveMessage = Message;
}

bool AArenaPlayerController::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}
void AArenaPlayerController::SetInfiniteAmmo(bool bEnable)
{
	bInfiniteAmmo = bEnable;
}

bool AArenaPlayerController::HasInfiniteClip() const
{
	return bInfiniteClip;
}
void AArenaPlayerController::SetInfiniteClip(bool bEnable)
{
	bInfiniteClip = bEnable;
}

bool AArenaPlayerController::HasHealthRegen() const
{
	return bHealthRegen;
}
void AArenaPlayerController::SetHealthRegen(bool bEnable)
{
	bHealthRegen = bEnable;
}

bool AArenaPlayerController::HasGodMode() const
{
	return bGodMode;
}
void AArenaPlayerController::SetGodMode(bool bEnable)
{
	bGodMode = bEnable;
}

bool AArenaPlayerController::IsMenuOpen() const
{
	return this->OpenMenu;
}
void AArenaPlayerController::SetMenu(bool bEnable)
{
	this->OpenMenu = bEnable;
	if (!OpenMenu && !OpenSettings && !OpenFriendsList)
	{
		bAllowGameActions = true;

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		FInputModeGameOnly Input;
		SetInputMode(Input);
	}
}

bool AArenaPlayerController::IsFriendsListOpen() const
{
	return this->OpenFriendsList;
}
void AArenaPlayerController::SetFriendsList(bool bEnable)
{
	this->OpenFriendsList = bEnable;
}

bool AArenaPlayerController::IsInventoryOpen() const
{
	return this->OpenInventory;
}
void AArenaPlayerController::SetInventory(bool bEnable)
{
	this->OpenInventory = bEnable;
	if (bEnable)
	{
		AArenaCharacter* Owner = Cast<AArenaCharacter>(GetPawnOrSpectator());
		if (Owner->GetPlayerState()->GetCombatState() == ECombatState::Aggressive)
		{
			Owner->OnToggleCombat();
		}
	}
}

bool AArenaPlayerController::IsNearbyInventory() const
{
	return this->NearbyInventory;
}
void AArenaPlayerController::SetNearbyInventory(bool bEnable)
{
	NearbyInventory = bEnable;
}

bool AArenaPlayerController::IsHUDOpen() const
{
	return this->OpenHUD;
}
void AArenaPlayerController::SetHUD(bool bEnable)
{
	this->OpenHUD = bEnable;
}

bool AArenaPlayerController::IsSettingsOpen() const
{
	return this->OpenSettings;
}
void AArenaPlayerController::SetSettings(bool bEnable)
{
	this->OpenSettings = bEnable;
	if (!OpenSettings)
	{
		bAllowGameActions = true;

		bShowMouseCursor = false;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		FInputModeGameOnly Input;
		SetInputMode(Input);
	}
}

bool AArenaPlayerController::IsMatchOver() const
{
	return this->MatchOver;
}
void AArenaPlayerController::SetMatchOver(bool bEnable)
{
	this->MatchOver = bEnable;
}

bool AArenaPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}
void AArenaPlayerController::SetAllowGameActions(bool bEnable)
{
	bAllowGameActions = bEnable;
}

/////////////////////////////////////////////////////////////// Server ///////////////////////////////////////////////////////////////

void AArenaPlayerController::HandleReturnToMainMenu()
{
	//OnHideScoreboard();
	CleanupSessionOnReturnToMenu();
}

void AArenaPlayerController::CleanupSessionOnReturnToMenu()
{
	UArenaGameInstance * AGI = GetWorld() != NULL ? Cast<UArenaGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (ensure(AGI != NULL))
	{
		AGI->CleanupSessionOnReturnToMenu();
	}
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

void AArenaPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AArenaPlayerState* CharacterState = Cast<AArenaPlayerState>(PlayerState);
	if (CharacterState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *CharacterState->SessionName.ToString());
				Sessions->StartSession(CharacterState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(FTimerDelegate::CreateUObject(this, &AArenaPlayerController::ClientStartOnlineGame_Implementation), 0.2f, false);
	}
}

void AArenaPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	AArenaPlayerState* CharacterState = Cast<AArenaPlayerState>(PlayerState);
	if (CharacterState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *CharacterState->SessionName.ToString());
				Sessions->EndSession(CharacterState->SessionName);
			}
		}
	}
}

void AArenaPlayerController::ClientReturnToMainMenu_Implementation(const FString& ReturnReason)
{
	CleanupSessionOnReturnToMenu();
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
	//SetIgnoreMoveInput(true);
	//bAllowGameActions = false;

	// Make sure that we still have valid view target
	SetViewTarget(GetPawn());
}

///////////////////////////////////////////////////////////////// Input /////////////////////////////////////////////////////////////////

void AArenaPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	// If we have a pawn we need to determine if we should show/hide the weapon
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(GetPawn());
	AArenaWeapon* MyWeapon = MyPawn ? MyPawn->GetCurrentWeapon() : NULL;
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

bool AArenaPlayerController::IsMoveInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsMoveInputIgnored();
	}
}

bool AArenaPlayerController::IsLookInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsLookInputIgnored();
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

bool AArenaPlayerController::GetAllowGameActions() const
{
	return bAllowGameActions;
}

TArray< class APlayerState* > AArenaPlayerController::GetPlayerArray()
{
	AArenaGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<AArenaGameState>() : NULL;
	if (MyGameState)
	{
		return MyGameState->PlayerArray;
	}
	TArray< class APlayerState* > Empty;
	return Empty;
}





