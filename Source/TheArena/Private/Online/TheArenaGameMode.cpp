// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

ATheArenaGameMode::ATheArenaGameMode(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/ArenaCharacter"));
	DefaultPawnClass = PlayerPawnOb.Class;

	//static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/ArenaAI"));
	//BotPawnClass = BotPawnOb.Class;

	static ConstructorHelpers::FClassFinder<AHUD> ArenaHUD(TEXT("/Game/UI/HUD/Blueprints/Arena_HUD"));
	//static ConstructorHelpers::FClassFinder<UUserWidget> ArenaHUD(TEXT("/Game/UI/Menu/ArenaHUD"));
	HUDClass = ArenaHUD.Class;

	PlayerControllerClass = AArenaPlayerController::StaticClass();
	PlayerStateClass = AArenaPlayerState::StaticClass();
	//SpectatorClass = AArenaSpectatorPawn::StaticClass();
	GameStateClass = AArenaGameState::StaticClass();

	MinRespawnDelay = 5.0f;

	bAllowBots = true;
}

FString ATheArenaGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void ATheArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 BotsCountOptionValue = GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);

	Super::InitGame(MapName, Options, ErrorMessage);
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

void ATheArenaGameMode::DefaultTimer()
{
	Super::DefaultTimer();

	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		return;
	}

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;

		if (MyGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();
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
	if (bAllowBots)
	{
		SpawnBotsForGame();
	}

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}

	// probably needs to be done somewhere else when Arenagame goes multiplayer
	TriggerRoundStartForLocalPlayers();
}

void ATheArenaGameMode::FinishMatch()
{
	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AArenaPlayerState* PlayerState = Cast<AArenaPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// probably needs to be done somewhere else when Arenagame goes multiplayer
		TriggerRoundEndForLocalPlayers();

		// lock all pawns
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void ATheArenaGameMode::TriggerRoundStartForLocalPlayers()
{
	// Send start match event, this will set the CurrentMap stat.
	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	if (Events.IsValid() && Identity.IsValid())
	{
		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
			ULocalPlayer* LocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : nullptr;
			if (LocalPlayer)
			{
				int32 UserIndex = LocalPlayer->ControllerId;
				if (UserIndex != -1)
				{
					FOnlineEventParms Params;
					Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1));
					Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0));

					if (PC->PlayerState->UniqueId.IsValid())
					{
						Events->TriggerEvent(*PC->PlayerState->UniqueId, TEXT("PlayerSessionStart"), Params);
					}
				}
			}
		}
	}
}

void ATheArenaGameMode::TriggerRoundEndForLocalPlayers()
{
	// Send start match event, this will set the CurrentMap stat.
	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	FOnlineEventParms Params;
	FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

	if (Events.IsValid() && Identity.IsValid())
	{
		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
			ULocalPlayer* LocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : nullptr;
			if (LocalPlayer)
			{
				int32 UserIndex = LocalPlayer->ControllerId;
				if (UserIndex != -1)
				{
					// round end
					{
						FOnlineEventParms Params;
						Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1));
						Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0));
						Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0));

						if (PC->PlayerState->UniqueId.IsValid())
						{
							Events->TriggerEvent(*PC->PlayerState->UniqueId, TEXT("PlayerSessionEnd"), Params);
						}
					}
				}
			}
		}
	}
}

void ATheArenaGameMode::RequestFinishAndExitToMainMenu()
{
	FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
	FString LocalReturnReason(TEXT(""));

	FinishMatch();

	APlayerController* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* Controller = *Iterator;
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientReturnToMainMenu(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	if (LocalPrimaryController != NULL)
	{
		LocalPrimaryController->ClientReturnToMainMenu(LocalReturnReason);
	}
}

void ATheArenaGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

bool ATheArenaGameMode::IsWinner(class AArenaPlayerState* PlayerState) const
{
	return false;
}

void ATheArenaGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	AArenaGameState* const MyGameState = Cast<AArenaGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	const FString EndGameError = TEXT("Match is over!");

	ErrorMessage = bMatchIsOver ? *EndGameError : GameSession->ApproveLogin(Options);
}


void ATheArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AArenaPlayerController* NewPC = Cast<AArenaPlayerController>(NewPlayer);

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
	}
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

UClass* ATheArenaGameMode::GetDefaultPawnClassForController(AController* InController)
{
	if (Cast<AArenaAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController(InController);
}

AActor* ATheArenaGameMode::ChoosePlayerStart(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestSpawn = PlayerStarts[i];
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

	APlayerStart* BestStart = NULL;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart(Player);
}

bool ATheArenaGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	/*AArenaTeamStart* ArenaSpawnPoint = Cast<AArenaTeamStart>(SpawnPoint);
	if (ArenaSpawnPoint)
	{
		AArenaAIController* AIController = Cast<AArenaAIController>(Player);
		if (ArenaSpawnPoint->bNotForBots && AIController)
		{
			return false;
		}

		if (ArenaSpawnPoint->bNotForPlayers && AIController == NULL)
		{
			return false;
		}
	}*/

	return false;
}

bool ATheArenaGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Player ? Cast<ACharacter>(Player->GetPawn()) : NULL;
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

	return true;
}

class AArenaAI* ATheArenaGameMode::SpawnBot(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (BotPawnClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bNoCollisionFail = true;
		AArenaAI* Bot = GetWorld()->SpawnActor<AArenaAI>(BotPawnClass, SpawnLocation, SpawnRotation, SpawnInfo);
		if (Bot)
		{
			Bot->SpawnDefaultController();
			return Bot;
		}
	}

	return NULL;
}

void ATheArenaGameMode::SpawnBotsForGame()
{
	// getting max number of players
	int32 MaxPlayers = -1;
	if (GameSession)
	{
		MaxPlayers = GameSession->MaxPlayers;
	}

	// checking number of human players
	int32 NumPlayers = 0;
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AArenaPlayerController* PC = Cast<AArenaPlayerController>(*It);
		if (PC)
		{
			++NumPlayers;
		}
	}

	// adding bots
	BotControllers.Empty();
	int32 NumBots = 0;
	while (NumPlayers < MaxPlayers && NumBots < MaxBots)
	{
		AArenaAI* Bot = SpawnBot(FVector(ForceInitToZero), FRotator(ForceInitToZero));
		if (Bot)
		{
			InitBot(Bot, NumBots + 1);
			++NumBots;
		}
	}
}

void ATheArenaGameMode::InitBot(AArenaAI* Bot, int BotNumber)
{
	AArenaAIController* AIPC = Bot ? Cast<AArenaAIController>(Bot->GetController()) : NULL;
	if (AIPC)
	{
		if (AIPC->PlayerState)
		{
			FString BotName = FString::Printf(TEXT("Bot %d"), BotNumber);
			AIPC->PlayerState->PlayerName = BotName;
		}
		AActor* BestStart = ChoosePlayerStart(AIPC);

		Bot->TeleportTo(BestStart->GetActorLocation(), BestStart->GetActorRotation(), false, true);
	}
}
