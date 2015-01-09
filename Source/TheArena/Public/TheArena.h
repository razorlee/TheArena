// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#ifndef __THEARENA_H__
#define __THEARENA_H__

#include "Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// The Arena Classes

#include "Public/AI/BTTask_FindPointNearEnemy.h"
#include "Public/AI/BTTask_HasLosTo.h"
#include "Public/AI/ArenaAIController.h"
#include "Public/ArenaTypes.h"
#include "Public/Player/ArenaCharacter.h"
#include "Public/AI/ArenaAI.h"
#include "Public/Weapons/ArenaDamageType.h"
#include "Public/Online/TheArenaGameMode.h"
#include "Public/Effects/ArenaImpactEffect.h"
#include "Public/Effects/ArenaExplosionEffect.h"
#include "Public/Player/ArenaPlayerCameraManager.h"
#include "Public/Player/ArenaCharacterMovement.h"
#include "Public/Player/ArenaPlayerController.h"
#include "Public/Player/ArenaPlayerController_Menu.h"
#include "Public/Player/ArenaLocalPlayer.h"
#include "Public/Online/ArenaGameSession.h"
#include "Public/Online/ArenaGameState.h"
#include "Public/Online/ArenaOnlineGameSettings.h"
#include "Public/Online/ArenaPlayerState.h"
#include "Public/Online/Arena_TeamDeathMatch.h"
#include "Public/Online/ArenaTeamStart.h"
#include "Public/Weapons/ArenaRangedWeapon.h"
#include "Public/Weapons/ArenaRangedWeapon_Projectile.h"
#include "Public/Weapons/ArenaProjectile.h"
#include "Public/Weapons/ArenaFragGrenade.h"
#include "Public/Weapons/ArenaRangedWeapon_Instant.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShooter, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogShooterWeapon, Log, All);

#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

#define MAX_PLAYER_NAME_LENGTH 16

#endif
