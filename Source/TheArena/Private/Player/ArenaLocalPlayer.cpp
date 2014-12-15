// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaLocalPlayer.h"


UArenaLocalPlayer::UArenaLocalPlayer(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

/*TSubclassOf<UOnlineSession> UArenaLocalPlayer::GetOnlineSessionClass()
{
	return UOnlineSessionClient::StaticClass();
}*/

/*UArenaPersistentUser* UArenaLocalPlayer::GetPersistentUser() const
{
	// if persistent data isn't loaded yet, load it
	if (PersistentUser == nullptr)
	{
		UArenaLocalPlayer* const MutableThis = const_cast<UArenaLocalPlayer*>(this);
		// casting away constness to enable caching implementation behavior
		MutableThis->LoadPersistentUser();
	}
	return PersistentUser;
}

void UArenaLocalPlayer::LoadPersistentUser()
{
	if (PersistentUser == NULL)
	{
		PersistentUser = UArenaPersistentUser::LoadPersistentUser(GetNickname(), ControllerId);
	}
}*/

void UArenaLocalPlayer::SetControllerId(int32 NewControllerId)
{
	ULocalPlayer::SetControllerId(NewControllerId);

	/*// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && (ControllerId != PersistentUser->GetUserIndex() || GetNickname() != PersistentUser->GetName()))
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (!PersistentUser)
	{
		LoadPersistentUser();
	}*/
}

