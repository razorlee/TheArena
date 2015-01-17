// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapons/ArenaRangedWeapon.h"
#include "ArenaSniperRifle.generated.h"

/**
 * 
 */
USTRUCT()
struct FSniperWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AArenaProjectile> ProjectileClass;

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float FiringSpreadMax;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float ProjectileLife;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 HitDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FSniperWeaponData()
	{
		ProjectileClass = NULL;
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		ProjectileLife = 10.0f;
		HitDamage = 10;
		ExplosionRadius = 1.0f;
		DamageType = UDamageType::StaticClass();
	}
};

UCLASS()
class THEARENA_API AArenaSniperRifle : public AArenaRangedWeapon
{
	GENERATED_BODY()

		AArenaSniperRifle(const class FObjectInitializer& ObjectInitializer);
	
	/** get current spread */
	float GetCurrentSpread() const;

	/** apply config on projectile */
	void ApplyWeaponConfig(FSniperWeaponData& Data);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EBullet;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FSniperWeaponData ProjectileConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
	
};
