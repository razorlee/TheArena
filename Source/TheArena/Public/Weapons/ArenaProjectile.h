#pragma once

#include "GameFramework/Actor.h"
#include "ArenaRangedWeapon.h"
#include "ArenaRangedWeapon_Projectile.h"
#include "Player/ArenaCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ArenaProjectile.generated.h"

UCLASS(Abstract, Blueprintable)
class THEARENA_API AArenaProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	void SetPawnOwner(class AArenaCharacter* MyPawn);

protected:

	/** pawn owner */
	UPROPERTY(Transient)
	class AArenaCharacter* MyPawn;

	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComp;

	/** effects for impact */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class AArenaImpactEffect> ImpactTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	struct FProjectileWeaponData WeaponConfig;

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
