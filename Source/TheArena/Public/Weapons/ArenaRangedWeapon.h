// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapons/ArenaWeapon.h"
#include "ArenaRangedWeapon.generated.h"

UCLASS()
class THEARENA_API AArenaRangedWeapon : public AArenaWeapon
{
	GENERATED_BODY()

	AArenaRangedWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void Destroyed() override;

////////////////////////////////////////// Input handlers //////////////////////////////////////////

	void StartAttack();
	void StopAttack();
	void StartReload();
	void StopReload();
	void StartMelee();
	void StopMelee();

///////////////////////////////////////// Action Functions /////////////////////////////////////////

	void HandleFiring();
	void FireWeapon();
	
	void Reload();

	void Melee();

	virtual void OnBurstStarted();
	virtual void OnBurstFinished();

///////////////////////////////////////// Aiming Helpers /////////////////////////////////////////

	FHitResult GetAdjustedAim();
	FVector GetCameraAim();
	FVector GetCameraDamageStartLocation(const FVector& AimDir);
	FVector GetMuzzleLocation();
	FVector GetMuzzleDirection();
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo);
	float GetCurrentSpread() const;

	void PlayAttackFX();
	void StopAttackFX();

	UFUNCTION()
	void SpawnProjectile(FVector Origin, FVector ShootDir, FHitResult Hit);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponState* GetWeaponState() override;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponAttributes* GetWeaponAttributes() override;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AArenaProjectile> ProjectileClass;

protected:

	class UArenaRangedWeaponState* WeaponState;

	UPROPERTY(EditDefaultsOnly, Category = Attributes)
	class UArenaRangedWeaponAttributes* WeaponAttributes;

	UPROPERTY(EditDefaultsOnly, Category = Attributes)
	class UArenaRangedWeaponEffects* WeaponEffects;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	FName MuzzleAttachPoint;
	
	/** Handle for efficient management of StopReload timer */
	FTimerHandle BurstFire;

	uint32 bRefiring;
};
