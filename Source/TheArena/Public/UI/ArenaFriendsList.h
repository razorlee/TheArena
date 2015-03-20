// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TheArena.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "Blueprint/UserWidget.h"
#include "ArenaFriendsList.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FServerEntry
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
	FString ServerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
	FString CurrentPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
	FString MaxPlayers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
	FString Ping;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
	int32 SearchResultsIndex;
};

UCLASS()
class THEARENA_API UArenaFriendsList : public UUserWidget//, public TSharedFromThis<UArenaFriendsList>
{
	GENERATED_BODY()

public:
	 
	/** Initialize menu */
	UFUNCTION(BlueprintCallable, Category = Online)
	void SetUp(UArenaGameInstance* _GameInstance, ULocalPlayer* _PlayerOwner);

	/** needed for every widget */
	void Construct();

	/**
	* Get the current game session
	*
	* @return The current game session
	*/
	AArenaGameSession* GetGameSession() const;

	/** Updates current search status */
	UFUNCTION(BlueprintCallable, Category = Online)
	void UpdateSearchStatus();

	/** Starts searching for servers */
	UFUNCTION(BlueprintCallable, Category = Online)
	void BeginServerSearch(bool bLANMatch, const FString& InMapFilterName); //

	/** Called when server search is finished */
	void OnServerSearchFinished();

	/** fill/update server list, should be called before showing this control */
	void UpdateServerList();

	/** selection changed handler */
	UFUNCTION(BlueprintCallable, Category = Online)
	void EntrySelectionChanged(int32 index);

	/** connect to chosen server */
	UFUNCTION(BlueprintCallable, Category = Online)
	void ConnectToServer(); 

	UFUNCTION(BlueprintCallable, Category = Online)
	TArray<FServerEntry> GetServerList();

	UFUNCTION(BlueprintCallable, Category = Online)
	void Search();

protected:

	/** Owning game instance */
	TWeakObjectPtr<UArenaGameInstance> GameInstance;

	/** Whether last searched for LAN (so spacebar works) */
	bool bLANMatchSearch;

	/** Whether we're searching for servers */
	bool bSearchingForServers;

	/** currently selected list item */
	TSharedPtr<FServerEntry> SelectedItem;

	/** current status text */
	FString StatusText;

	/** Map filter name to use during server searches */
	FString MapFilterName;

	/** action bindings array */
	TArray< TSharedPtr<FServerEntry> > ServerList;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** Delegate function executed after checking privileges for joining an online game */
	void OnUserCanPlayOnlineJoin(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** Start the check for whether the owner of the menu has online privileges */
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate);

	/** Common cleanup code for any Privilege task delegate */
	void CleanupOnlinePrivilegeTask();

	/** Join server */
	UFUNCTION(BlueprintCallable, Category = Online)
	void OnJoinServer();

};
