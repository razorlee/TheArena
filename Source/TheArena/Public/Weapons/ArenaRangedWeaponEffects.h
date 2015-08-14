// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaRangedWeaponEffects.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaRangedWeaponEffects : public UActorComponent
{
	GENERATED_BODY()

public:	

	UArenaRangedWeaponEffects();

	virtual void BeginPlay() override;
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

//////////////////////////////////////// Effects ////////////////////////////////////////

	UFUNCTION()
	UParticleSystem* GetMuzzleFX();
	UFUNCTION()
	void SetMuzzleFX(UParticleSystem* Value);

	UFUNCTION()
	FName GetTrailFXParams();
	UFUNCTION()
	void SetTrailFXParams(FName Value);

	UFUNCTION()
	UParticleSystem* GetTrailFX();
	UFUNCTION()
	void SetTrailFX(UParticleSystem* Value);

	UFUNCTION()
	UParticleSystemComponent* GetMuzzlePSC();
	UFUNCTION()
	void SetMuzzlePSC(UParticleSystemComponent* Value);

	UFUNCTION()
	TSubclassOf<UCameraShake> GetCameraShake();
	UFUNCTION()
	void SetCameraShake(TSubclassOf<UCameraShake> Value);

//////////////////////////////////////// Audio ////////////////////////////////////////

	UFUNCTION()
	UAudioComponent* GetFireAC();
	UFUNCTION()
	void SetFireAC(UAudioComponent* Value);

	UFUNCTION()
	USoundCue* GetFireStartSound();
	UFUNCTION()
	void SetFireStartSound(USoundCue* Value);

	UFUNCTION()
	USoundCue* GetFireLoopSound();
	UFUNCTION()
	void SetFireLoopSound(USoundCue* Value);

	UFUNCTION()
	USoundCue* GetFireFinishSound();
	UFUNCTION()
	void SetFireFinishSound(USoundCue* Value);

	UFUNCTION()
	USoundCue* GetOutOfAmmoSound();
	UFUNCTION()
	void SetOutOfAmmoSound(USoundCue* Value);

	UFUNCTION()
	USoundCue* GetReloadSound();
	UFUNCTION()
	void SetReloadSound(USoundCue* Value);

	UFUNCTION()
	USoundCue* GetMeleeSound();
	UFUNCTION()
	void SetMeleeSound(USoundCue* Value);

//////////////////////////////////////// Animations ////////////////////////////////////////

	UFUNCTION()
	bool IsPlayingFireAnim();
	UFUNCTION()
	void SetPlayingFireAnim(bool Value);

	UFUNCTION()
	UAnimMontage* GetReloadAnim();
	UFUNCTION()
	void SetReloadAnim(UAnimMontage* Value);

	UFUNCTION()
	UAnimMontage* GetMeleeAnim();
	UFUNCTION()
	void SetMeleeAnim(UAnimMontage* Value);

	UFUNCTION()
	UAnimMontage* GetFireAnim();
	UFUNCTION()
	void SetFireAnim(UAnimMontage* Value);

protected:

//////////////////////////////////////// Effects ////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** camera shake on firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

//////////////////////////////////////// Audio ////////////////////////////////////////

	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireStartSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* MeleeSound;

//////////////////////////////////////// Animations ////////////////////////////////////////

	UPROPERTY()
	bool PlayingFireAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* FireAnim;
	
};
