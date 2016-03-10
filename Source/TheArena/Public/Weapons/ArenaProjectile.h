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
	UFUNCTION(BlueprintCallable, Category = Projectile)

	void InitVelocity(FVector& ShootDirection);

	/** setup collision */
	UFUNCTION(BlueprintCallable, Category = Projectile)

	void SetCollisionChannel(ECollisionChannel Value);

	/** handle hit */
	UFUNCTION(BlueprintCallable, Category = Projectile)
	void OnImpact(const FHitResult& HitResult);

	/** trigger explosion */
	UFUNCTION(BlueprintCallable, Category = Projectile)
	void Explode(const FHitResult& Impact);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetInitialSpeed(float Speed);

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	void SetPawnOwner(class AArenaCharacter* MyPawn);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetHitResults(const FHitResult& Impact);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetDamage(float Value);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetIsExplosive(bool Value);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetExplosionRadius(float Value);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void SetIsAffectByVelocity(bool Value);

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void StartTimer();
	UFUNCTION(BlueprintCallable, Category = Projectile)
	float StopTimer();

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

	/** pawn owner */
	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
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

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float Damage;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool IsExplosive;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool IsAffectByVelocity;

	UPROPERTY()
	float StartTime;

	UPROPERTY()
	float StopTime;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

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
