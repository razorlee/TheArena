// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaExplosionEffect.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class THEARENA_API AArenaExplosionEffect : public AActor
{
	GENERATED_BODY()

	AArenaExplosionEffect(const class FObjectInitializer& PCIP);

public: 

	/** explosion FX */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* ExplosionFX;

	/** explosion light */
	UPROPERTY(VisibleDefaultsOnly, Category = Effect)
	class UPointLightComponent* ExplosionLight;

	/** how long keep explosion light on? */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	float ExplosionLightFadeOut;

	/** explosion sound */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	USoundCue* ExplosionSound;

	/** explosion decals */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	struct FDecalData Decal;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category = Surface)
	FHitResult SurfaceHit;

	/** spawn explosion */
	virtual void BeginPlay() override;

	/** update fading light */
	virtual void Tick(float DeltaSeconds) override;

private:

	/** Point light component name */
	FName ExplosionLightComponentName;
	
};
