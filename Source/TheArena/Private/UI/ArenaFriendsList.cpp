// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaFriendsList.h"
#include "OnlineSubsystemUtils.h"

UArenaFriendsList::UArenaFriendsList(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//This sets the Delegate so we can be notified when the subsystem has completed reading the friends list.
	, ReadCompleteDelegate(FOnReadFriendsListCompleteDelegate::CreateUObject(this, &ThisClass::OnReadFriendsListComplete))
{
}

UArenaFriendsList* UArenaFriendsList::GetFriendsList(UObject* WorldContextObject, class APlayerController* PlayerController)
{
	UArenaFriendsList* Proxy = NewObject<UArenaFriendsList>();
	Proxy->PlayerControllerWeakPtr = PlayerController;

	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void UArenaFriendsList::Activate(UArenaGameInstance* _GameInstance)
{
	//This gets the OnlineSubsystem
	GameInstance = _GameInstance;
	auto Friends = Online::GetFriendsInterface();
	if (Friends.IsValid())
	{
		ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);
		Friends->AddOnReadFriendsListCompleteDelegate(Player->ControllerId, ReadCompleteDelegate);
		Friends->ReadFriendsList(Player->ControllerId, EFriendsLists::ToString(EFriendsLists::Default));

		return;
	}

	// Fail immediately
	TArray<FBlueprintFriend> Fail;
	OnFailure.Broadcast(Fail);
}

void UArenaFriendsList::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		auto Friends = Online::GetFriendsInterface();
		if (Friends.IsValid())
		{
			TArray<FBlueprintFriend>FriendsArr;
			ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);
			TArray< TSharedRef<FOnlineFriend> > OutFriends;

			Friends->GetFriendsList(LocalUserNum, ListName, OutFriends);
			for (int32 i = 0; i < OutFriends.Num(); i++)
			{
				TSharedRef<FOnlineFriend> Friend = OutFriends[i];
				//This creates a BlueprintFriend, a custom class for blueprints, because I could not get anything other to work
				FBlueprintFriend BPFriend = FBlueprintFriend();

				BPFriend.Presence = EOnlinePresenceState::ToString(Friend->GetPresence().Status.State);
				BPFriend.DisplayName = Friend->GetDisplayName();
				BPFriend.RealName = Friend->GetRealName();
				BPFriend.UniqueNetId = Friend->GetUserId()->ToString();
				FriendsArr.Add(BPFriend);
			}
			Friends->ClearOnReadFriendsListCompleteDelegate(LocalUserNum, ReadCompleteDelegate);
			OnSuccess.Broadcast(FriendsArr);
		}
	}

	else
	{
		TArray<FBlueprintFriend> Fail;
		OnFailure.Broadcast(Fail);
	}
}