#pragma once

#include "Runtime/Online/OnlineSubsystem/Public/OnlineSessionSettings.h"

class FArenaOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FArenaOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FArenaOnlineSessionSettings() {}
};


class FArenaOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FArenaOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FArenaOnlineSearchSettings() {}
};


class FArenaOnlineSearchSettingsEmptyDedicated : public FArenaOnlineSearchSettings
{
public:
	FArenaOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FArenaOnlineSearchSettingsEmptyDedicated() {}
};