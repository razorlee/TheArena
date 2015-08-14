// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapons/ArenaWeapon.h"
#include "ArenaRangedWeapon.generated.h"

UCLASS()
class THEARENA_API AArenaRangedWeapon : public AArenaWeapon
{
	GENERATED_BODY()

public :

	AArenaRangedWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;

	virtual void Destroyed() override;

////////////////////////////////////////// Input handlers //////////////////////////////////////////

	void StartAttack();
	void StopAttack();

	void StartReload();
	UFUNCTION(NetMulticast, Reliable)
	void StopReload();

///////////////////////////////////////// Action Functions /////////////////////////////////////////

	void HandleFiring();
	void FireWeapon();

	void HandleRecoil(float DeltaSeconds);
	void FinishRecoil(float DeltaSeconds);

	void HandleBurst();

	UFUNCTION(NetMulticast, Reliable)
	void Reload();
	UFUNCTION(NetMulticast, Reliable)
	void FinishReload();

	virtual void OnBurstStarted();
	virtual void OnBurstFinished();

///////////////////////////////////////// Aiming Helpers /////////////////////////////////////////

	void SpawnTrailEffect(const FVector& EndPoint);
	FHitResult GetAdjustedAim();
	FVector GetCameraAim();
	FVector GetCameraDamageStartLocation(const FVector& AimDir);
	FVector GetMuzzleLocation();
	FVector GetMuzzleDirection();
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo);
	float GetCurrentSpread() const;

	virtual void PlayAttackFX();
	virtual void StopAttackFX();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponState* GetWeaponState() override;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponAttributes* GetWeaponAttributes() override;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponEffects* GetWeaponEffects() override;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AArenaProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadWrite, Category = Mesh)
	UMaterialInstanceDynamic* MaterialInstanceA;

	UPROPERTY(BlueprintReadWrite, Category = Mesh)
	UMaterialInstanceDynamic* MaterialInstanceB;

protected:

	class UArenaRangedWeaponState* WeaponState;

	UPROPERTY(EditDefaultsOnly, Category = Attributes)
	class UArenaRangedWeaponAttributes* WeaponAttributes;

	UPROPERTY(EditDefaultsOnly, Category = Attributes)
	class UArenaRangedWeaponEffects* WeaponEffects;

	UPROPERTY(BlueprintReadWrite, Category = Config)
	TEnumAsByte< ECollisionChannel > Channel;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	FName MuzzleAttachPoint;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	float BurstCounter;

	float Recoil;
	float RecoilCounter;

	bool IsRecoiling;

	FTimerHandle BurstFire;

	uint32 bRefiring;

///////////////////////////////////////////// Replication /////////////////////////////////////////////

	UFUNCTION()
	void OnRep_BurstCounter();

/////////////////////////////////////////////// Server ///////////////////////////////////////////////

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartAttack();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStopAttack();

	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSpawnProjectile(FVector Origin, FVector ShootDir, FHitResult Hit);

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();
};
