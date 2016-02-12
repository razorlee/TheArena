#pragma once

#include "Online.h"
#include "Online/ArenaOnlineGameSettings.h"
#include "GameFramework/PlayerController.h"
#include "ArenaPlayerController.generated.h"

UCLASS(config=game)
class THEARENA_API AArenaPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:

	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/** Delegate for starting a session */
	FOnEndSessionCompleteDelegate OnStartSessionCompleteEndItDelegate;

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);

	/** used for input simulation from blueprint (for automatic perf tests) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SimulateInputKey(FKey Key, bool bPressed = true);

	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);

	UFUNCTION(exec)
	void ChangeTeam();
	UFUNCTION(NetMultiCast, Reliable)
	void FinishChangeTeam(AArenaPlayerState* CharacterState);

	void OnToggleInGameMenu();

	UFUNCTION(BlueprintCallable, Category = Menu)
	void OnToggleInventory();

	UFUNCTION(BlueprintCallable, Category = Menu)
	void OnToggleMatchmaking();

	UFUNCTION(BlueprintCallable, Category = HUD)
	FText GetInteractiveMessage();
	UFUNCTION(BlueprintCallable, Category = HUD)
	void SetInteractiveMessage(FText Message);

	bool HasGodMode() const;
	UFUNCTION(exec)
	void SetGodMode(bool bEnable);

	bool HasInfiniteAmmo() const;
	void SetInfiniteAmmo(bool bEnable);

	bool HasInfiniteClip() const;
	void SetInfiniteClip(bool bEnable);

	bool HasHealthRegen() const;
	void SetHealthRegen(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsMenuOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetMenu(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsFriendsListOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetFriendsList(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsInventoryOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetInventory(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsMatchmakingOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetMatchmaking(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsNearbyInventory() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetNearbyInventory(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsNearbyMatchmaking() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetNearbyMatchmaking(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsHUDOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetHUD(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsSettingsOpen() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetSettings(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsMatchOver() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetMatchOver(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsGameInputAllowed() const;
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetAllowGameActions(bool bEnable);

	void CleanupSessionOnReturnToMenu();

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool GetAllowGameActions() const;

	virtual void OnStartSessionCompleteEndIt(FName SessionName, bool bWasSuccessful);

	virtual void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Returns true if movement input is ignored. Overridden to always allow spectators to move. */
	virtual bool IsMoveInputIgnored() const override;

	/** Returns true if look input is ignored. Overridden to always allow spectators to look around. */
	virtual bool IsLookInputIgnored() const override;

	virtual void InitInputSystem() override;

	/** become a spectator */
	void EnterSpectatorMode();

	void OnKill();

	/** Cleans up any resources necessary to return to main menu.  Does not modify GameInstance state. */
	virtual void HandleReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = HUD)
	TArray< class APlayerState* > GetPlayerArray();

	UPROPERTY(BlueprintReadWrite, Category = Multiplayer)
	bool IsWinner;

protected:

	FTimerHandle StartGame_Timer;

	UPROPERTY()
	FText InteractiveMessage;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenMenu : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenFriendsList : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenInventory : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenMatchmaking : 1;

	UPROPERTY(Transient, Replicated)
	uint8 NearbyInventory : 1;

	UPROPERTY(Transient, Replicated)
	uint8 NearbyMatchmaking : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenHUD : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenSettings : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 MatchOver : 1;

	/** infinite ammo cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteAmmo : 1;

	/** infinite clip cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteClip : 1;

	/** health regen cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bHealthRegen : 1;

	/** god mode cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bGodMode : 1;

	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	uint8 bAllowGameActions : 1;

	/** true for the first frame after the game has ended */
	uint8 bGameEndedFrame : 1;

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	//Begin AActor interface

	/** after all game elements are created */
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	//End AActor interface

	//Begin AController interface

	/** transition to dead state, retries spawning later */
	virtual void FailedToSpawnPawn() override;

	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* P) override;

	//End AController interface

	// Begin APlayerController interface

	/** respawn after dying */
	virtual void UnFreeze() override;

	/** sets up input */
	virtual void SetupInputComponent() override;

	/**
	* Called from game info upon end of the game, used to transition to proper state.
	*
	* @param EndGameFocus Actor to set as the view target on end game
	* @param bIsWinner true if this controller is on winning team
	*/
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;

	/** Return the client to the main menu gracefully */
	void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;

	/** Causes the player to commit suicide */
	UFUNCTION(exec)
	virtual void Suicide();

	/** Notifies the server that the client has suicided */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSuicide();

	// End APlayerController interface

	FName ServerSayString;

	// Timer used for updating friends in the player tick.
	float ArenaFriendUpdateTimer;

	// For tracking whether or not to send the end event
	bool bHasSentStartEvents;

	void Reset() override;

	UFUNCTION(reliable, server, WithValidation)
	void ServerChangeTeam();

};

