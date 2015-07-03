#pragma once

#include "GameFramework/Actor.h"
#include "ArenaRangedWeapon.h"
#include "Player/ArenaCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ArenaProjectile.generated.h"

USTRUCT()
struct FProjectileHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	bool bExploded;

	UPROPERTY()
	FHitResult Hit;
};

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

	void SetHitResults(const FHitResult& Impact);

private:

	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComp;

protected:

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	bool IsExplosive;

	/** pawn owner */
	UPROPERTY(Transient)
	class AArenaCharacter* MyPawn;

	FHitResult HitResults;

	/** effects for impact */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class AArenaImpactEffect> ImpactTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** explode hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	FProjectileHitInfo ExplodeNotify;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** find physical material */
	FHitResult ProjectileTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
};
