// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TheArena.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "Blueprint/UserWidget.h"
#include "ArenaFriendsList.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FBlueprintFriend
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString RealName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString Presence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Friend")
	FString UniqueNetId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGetFriendsListDelegate, const TArray<FBlueprintFriend>&, Results);

UCLASS()
class THEARENA_API UArenaFriendsList : public UUserWidget
{
	GENERATED_BODY()

	UArenaFriendsList(const FObjectInitializer& ObjectInitializer);

public:
	/** Owning game instance */
	TWeakObjectPtr<UArenaGameInstance> GameInstance;

	UPROPERTY(BlueprintAssignable)
	FBlueprintGetFriendsListDelegate OnSuccess;

	// Called when there was an error creating the session
	UPROPERTY(BlueprintAssignable)
	FBlueprintGetFriendsListDelegate OnFailure;


	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online|Friends")
	static  UArenaFriendsList* GetFriendsList(UObject* WorldContextObject, class APlayerController* PlayerController);

	// UOnlineBlueprintCallProxyBase interface
	virtual void Activate(UArenaGameInstance* _GameInstance);

	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	UObject* WorldContextObject;

private:
	// Internal callback when session creation completes, calls StartSession
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	FOnReadFriendsListCompleteDelegate ReadCompleteDelegate;
};
