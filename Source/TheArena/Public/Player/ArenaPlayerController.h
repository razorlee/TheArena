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

	/** notify player about finished match */
	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);

	/** used for input simulation from blueprint (for automatic perf tests) */
	//UFUNCTION(BlueprintCallable, Category = "Input")
	//void SimulateInputKey(FKey Key, bool bPressed = true);

	/* Overriden Message implementation. */
	//virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime) override;

	/** Local function run an emote */
	// 	UFUNCTION(exec)
	// 	virtual void Emote(const FString& Msg);

	/** toggle InGameMenu handler */
	void OnToggleInGameMenu();

	/** set infinite ammo cheat */
	void SetInfiniteAmmo(bool bEnable);

	/** set infinite clip cheat */
	void SetInfiniteClip(bool bEnable);

	/** set health regen cheat */
	void SetHealthRegen(bool bEnable);

	/** set god mode cheat */
	UFUNCTION(exec)
	void SetGodMode(bool bEnable);

	/** set open menu */
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetMenu(bool bEnable);

	/** set open menu */
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetFriendsList(bool bEnable);

	/** set open menu */
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetHUD(bool bEnable);

	/** set open menu */
	UFUNCTION(BlueprintCallable, Category = Menu)
	void SetAllowGameActions(bool bEnable);

	/** get infinite ammo cheat */
	bool HasInfiniteAmmo() const;

	/** get infinite clip cheat */
	bool HasInfiniteClip() const;

	/** get health regen cheat */
	bool HasHealthRegen() const;

	/** get gode mode cheat */
	bool HasGodMode() const;

	/** check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsGameInputAllowed() const;

	/** Ends and/or destroys game session */
	void CleanupSessionOnReturnToMenu();

	/** Returns whether the menu is open */
	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsMenuOpen() const;

	/** Returns whether the friends list is open */
	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsFriendsListOpen() const;

	/** Returns whether the HUD is open */
	UFUNCTION(BlueprintCallable, Category = Menu)
	bool IsHUDOpen() const;

	UFUNCTION(BlueprintCallable, Category = Menu)
	bool GetAllowGameActions() const;

	/**
	* Delegate fired when starting an online session has completed (intends to end it, but has to wait for the start to complete first)
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnStartSessionCompleteEndIt(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when ending an online session has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	* Delegate fired when destroying an online session has completed
	*
	* @param SessionName the name of the session this callback is for
	* @param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	// Begin APlayerController interface

	/** handle weapon visibility */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** initialize the input system from the player settings */
	virtual void InitInputSystem() override;

	// End APlayerController interface

	// begin AArenaPlayerController-specific

	/** Informs that player fragged someone */
	void OnKill();

	// end AArenaPlayerController-specific

protected:

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenMenu : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenFriendsList : 1;

	/** informs the HUD if the menu is open */
	UPROPERTY(Transient, Replicated)
	uint8 OpenHUD : 1;

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

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** try to find spot for death cam */
	//bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

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
};

