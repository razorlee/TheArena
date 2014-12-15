// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapons/ArenaRangedWeapon.h"
#include "ArenaSniperRifle.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaSniperRifle : public AArenaRangedWeapon
{
	GENERATED_UCLASS_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
		float weight;
	
};
