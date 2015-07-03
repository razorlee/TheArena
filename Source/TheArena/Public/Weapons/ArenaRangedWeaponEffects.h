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
	bool IsLoopedMuzzleFX();
	UFUNCTION()
	void SetLoopedMuzzleFX(bool Value);

	UFUNCTION()
	UParticleSystem* GetMuzzleFX();
	UFUNCTION()
	void SetMuzzleFX(UParticleSystem* Value);

	UFUNCTION()
	UParticleSystemComponent* GetMuzzlePSC();
	UFUNCTION()
	void SetMuzzlePSC(UParticleSystemComponent* Value);

//////////////////////////////////////// Audio ////////////////////////////////////////

	UFUNCTION()
	UAudioComponent* GetFireAC();
	UFUNCTION()
	void SetFireAC(UAudioComponent* Value);

	UFUNCTION()
	bool IsLoopedFireSound();
	UFUNCTION()
	void SetLoopedFireSound(bool Value);

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
	bool IsLoopedFireAnim();
	UFUNCTION()
	void SetLoopedFireAnim(bool Value);

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
	bool LoopedMuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

//////////////////////////////////////// Audio ////////////////////////////////////////

	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	bool LoopedFireSound;

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

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	bool LoopedFireAnim;

	UPROPERTY()
	bool PlayingFireAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* MeleeAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* FireAnim;
	
};
