// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaFragGrenade.generated.h"

/**
 * 
 */
USTRUCT()
struct FGrenadeWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** life time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float ProjectileLife;

	/** damage at impact point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FGrenadeWeaponData()
	{
		ProjectileLife = 4.0f;
		ExplosionDamage = 1000.0f;
		ExplosionRadius = 500.0f;
		DamageType = UDamageType::StaticClass();
	}
};

UCLASS(Abstract, Blueprintable)
class THEARENA_API AArenaFragGrenade : public AActor
{
	GENERATED_BODY()

	AArenaFragGrenade(const class FObjectInitializer& PCIP);

public:

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

protected:

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Projectile)
	FGrenadeWeaponData Explosion;

	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class UParticleSystemComponent* ParticleComp;

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class AArenaExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	struct FGrenadeWeaponData GrenadeConfig;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;
	
};
