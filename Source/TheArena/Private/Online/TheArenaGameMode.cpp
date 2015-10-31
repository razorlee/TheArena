// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

ATheArenaGameMode::ATheArenaGameMode(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/BP_ArenaCharacter"));
	static ConstructorHelpers::FClassFinder<ASpectatorPawn> SpectatorPawnOb(TEXT("/Game/Blueprints/Pawns/BP_ArenaSpectator"));
	static ConstructorHelpers::FClassFinder<AHUD> ArenaHUD(TEXT("/Game/UI/HUD/Blueprints/Arena_HUD"));
	
	//static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/ArenaAI"));
	//BotPawnClass = BotPawnOb.Class;

	DefaultPawnClass = PlayerPawnOb.Class;
	HUDClass = ArenaHUD.Class;
	PlayerControllerClass = AArenaPlayerController::StaticClass();
	PlayerStateClass = AArenaPlayerState::StaticClass();
	SpectatorClass = SpectatorPawnOb.Class;
	GameStateClass = AArenaGameState::StaticClass();

	MinRespawnDelay = 0.0f;

	bAllowBots = false;
	bTeamEliminated = false;
	bUseSeamlessTravel = true;
}

FString ATheArenaGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void ATheArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);
	Super::InitGame(MapName, Options, ErrorMessage);

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && Cast<UArenaGameInstance>(GameInstance)->GetIsOnline())
	{
		bPauseable = false;
	}
}

void ATheArenaGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots)
{
	bAllowBots = bInAllowBots;
	MaxBots = InMaxBots;
}

/** Returns game session class to use */
TSubclassOf<AGameSession> ATheArenaGameMode::GetGameSessionClass() const
{
	return AArenaGameSession::StaticClass();
}

void ATheArenaGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ATheArenaGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ATheArenaGameMode::DefaultTimer()
{
	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		//return;
	}

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;
		if (MyGameState->RemainingTime <= 0 || bTeamEliminated == true)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();

				// Send end round events
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					AArenaPlayerController* PlayerController = Cast<AArenaPlayerController>(*It);

					if (PlayerController && MyGameState)
					{
						AArenaPlayerState* PlayerState = Cast<AArenaPlayerState>((*It)->PlayerState);
						const bool bIsWinner = IsWinner(PlayerState);

						PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
					}
				}
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

void ATheArenaGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	MyGameState->RemainingTime = RoundTime;

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}
}

void ATheArenaGameMode::HandleMatchIsWaitingToStart()
{
	if (bDelayedStart)
	{
		// start warmup if needed
		AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && WarmupTime > 0)
			{
				MyGameState->RemainingTime = WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 0.0f;
			}
		}
	}
}

void ATheArenaGameMode::FinishMatch()
{
	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		if (Role == ROLE_Authority)
		{
			DetermineMatchWinner();
		}

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AArenaPlayerState* PlayerState = Cast<AArenaPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		bTeamEliminated = false;
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void ATheArenaGameMode::RequestFinishAndExitToMainMenu()
{
	FinishMatch();

	UArenaGameInstance* const GameInstance = Cast<UArenaGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		//GameInstance->RemoveSplitScreenPlayers();
	}

	AArenaPlayerController* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AArenaPlayerController* Controller = Cast<AArenaPlayerController>(*Iterator);

		if (Controller == NULL)
		{
			continue;
		}

		if (!Controller->IsLocalController())
		{
			const FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
			//Controller->ClientReturnToMainMenu(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	// GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
	if (LocalPrimaryController != NULL)
	{
		//LocalPrimaryController->HandleReturnToMainMenu();
	}
}

void ATheArenaGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

void ATheArenaGameMode::CheckTeamElimination()
{
	// nothing to do here
}

bool ATheArenaGameMode::IsWinner(class AArenaPlayerState* PlayerState) const
{
	return false;
}

void ATheArenaGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<const FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	if (bMatchIsOver)
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


void ATheArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AArenaPlayerController* NewPC = Cast<AArenaPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}
}

void ATheArenaGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	AArenaPlayerState* KillerPlayerState = Killer ? Cast<AArenaPlayerState>(Killer->PlayerState) : NULL;
	AArenaPlayerState* VictimPlayerState = KilledPlayer ? Cast<AArenaPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
	CheckTeamElimination();
}

float ATheArenaGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AArenaCharacter* DamagedPawn = Cast<AArenaCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AArenaPlayerState* DamagedPlayerState = Cast<AArenaPlayerState>(DamagedPawn->PlayerState);
		AArenaPlayerState* InstigatorPlayerState = Cast<AArenaPlayerState>(EventInstigator->PlayerState);

		// disable friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.0f;
		}

		// scale self instigated damage
		if (InstigatorPlayerState == DamagedPlayerState)
		{
			ActualDamage *= DamageSelfScale;
		}
	}

	return ActualDamage;
}

bool ATheArenaGameMode::CanDealDamage(class AArenaPlayerState* DamageInstigator, class AArenaPlayerState* DamagedPlayer) const
{
	return true;
}

bool ATheArenaGameMode::AllowCheats(APlayerController* P)
{
	return false;
}

bool ATheArenaGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

UClass* ATheArenaGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* ATheArenaGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	APlayerStart* BestStart = NULL;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		if (TestSpawn->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}


	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool ATheArenaGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	AArenaTeamStart* ArenaSpawnPoint = Cast<AArenaTeamStart>(SpawnPoint);
	if (ArenaSpawnPoint)
	{
		if (ArenaSpawnPoint->bNotForPlayers)
		{
			return false;
		}
		return true;
	}

	return false;
}

bool ATheArenaGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());
	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

void ATheArenaGameMode::RestartGame()
{
	//Super::RestartGame();

	GetWorld()->ServerTravel("/Game/Maps/TheArenaPrototype3", true, true);
}