// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#ifndef __THEARENA_H__
#define __THEARENA_H__

#include "Engine.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealNetwork.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// The Arena Classes

#include "Public/ArenaGameInstance.h"
#include "Public/ArenaTypes.h"
#include "Public/Player/ArenaCharacter.h"
#include "Public/Weapons/ArenaDamageType.h"
#include "Public/Online/TheArenaGameMode.h"
#include "Public/Effects/ArenaImpactEffect.h"
#include "Public/Effects/ArenaExplosionEffect.h"
#include "Public/Player/ArenaPlayerCameraManager.h"
#include "Public/Player/ArenaCharacterMovement.h"
#include "Public/Player/ArenaCharacterAttributes.h"
#include "Public/Player/ArenaCharacterEquipment.h"
#include "Public/Player/ArenaPlayerController.h"
#include "Public/Player/ArenaPlayerController_Menu.h"
#include "Public/Player/ArenaLocalPlayer.h"
#include "Public/Player/ArenaSaveGame.h"
#include "Public/Player/ArenaCharacterCan.h"
#include "Public/Online/ArenaGameSession.h"
#include "Public/Online/ArenaGameState.h"
#include "Public/Online/ArenaOnlineGameSettings.h"
#include "Public/Online/ArenaPlayerState.h"
#include "Public/Online/Arena_TeamDeathMatch.h"
#include "Public/Online/ArenaTeamStart.h"
#include "Public/UI/ArenaFriendsList.h"
#include "Public/UI/ArenaStartMenu.h"
#include "Public/Weapons/ArenaWeapon.h"
#include "Public/Weapons/ArenaWeaponCan.h"
#include "Public/Weapons/ArenaRangedWeapon.h"
#include "Public/Weapons/ArenaRangedWeaponState.h"
#include "Public/Weapons/ArenaRangedWeaponAttributes.h"
#include "Public/Weapons/ArenaRangedWeaponEffects.h"
#include "Public/Weapons/ArenaProjectile.h"
#include "Public/Weapons/ArenaFragGrenade.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShooter, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogShooterWeapon, Log, All);

#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

#define MAX_PLAYER_NAME_LENGTH 16

#endif
