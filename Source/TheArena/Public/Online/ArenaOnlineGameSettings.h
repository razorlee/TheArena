// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
* General session settings for a arena game
*/
class FArenaOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FArenaOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FArenaOnlineSessionSettings() {}
};

/**
* General search setting for a arena game
*/
class FArenaOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FArenaOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FArenaOnlineSearchSettings() {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
class FArenaOnlineSearchSettingsEmptyDedicated : public FArenaOnlineSearchSettings
{
public:
	FArenaOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FArenaOnlineSearchSettingsEmptyDedicated() {}
};