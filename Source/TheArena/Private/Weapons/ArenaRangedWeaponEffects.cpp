// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeaponEffects.h"


UArenaRangedWeaponEffects::UArenaRangedWeaponEffects()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UArenaRangedWeaponEffects::BeginPlay()
{
	Super::BeginPlay();	

	//LoopedMuzzleFX = false;
	//LoopedFireSound = false;
	//LoopedFireAnim = false;
	//PlayingFireAnim = false;
}

void UArenaRangedWeaponEffects::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

//////////////////////////////////////// Effects ////////////////////////////////////////

bool UArenaRangedWeaponEffects::IsLoopedMuzzleFX()
{
	return LoopedMuzzleFX;
}
void UArenaRangedWeaponEffects::SetLoopedMuzzleFX(bool Value)
{
	LoopedMuzzleFX = Value;
}

UParticleSystem* UArenaRangedWeaponEffects::GetMuzzleFX()
{
	return MuzzleFX;
}
void UArenaRangedWeaponEffects::SetMuzzleFX(UParticleSystem* Value)
{
	MuzzleFX = Value;
}

UParticleSystemComponent* UArenaRangedWeaponEffects::GetMuzzlePSC()
{
	return MuzzlePSC;
}
void UArenaRangedWeaponEffects::SetMuzzlePSC(UParticleSystemComponent* Value)
{
	MuzzlePSC = Value;
}

//////////////////////////////////////// Audio ////////////////////////////////////////

UAudioComponent* UArenaRangedWeaponEffects::GetFireAC()
{
	return FireAC;
}
void UArenaRangedWeaponEffects::SetFireAC(UAudioComponent* Value)
{
	FireAC = Value;
}

bool UArenaRangedWeaponEffects::IsLoopedFireSound()
{
	return LoopedFireSound;
}
void UArenaRangedWeaponEffects::SetLoopedFireSound(bool Value)
{
	LoopedFireSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetFireStartSound()
{
	return FireStartSound;
}
void UArenaRangedWeaponEffects::SetFireStartSound(USoundCue* Value)
{
	FireStartSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetFireLoopSound()
{
	return FireLoopSound;
}
void UArenaRangedWeaponEffects::SetFireLoopSound(USoundCue* Value)
{
	FireLoopSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetFireFinishSound()
{
	return FireFinishSound;
}
void UArenaRangedWeaponEffects::SetFireFinishSound(USoundCue* Value)
{
	FireFinishSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetOutOfAmmoSound()
{
	return OutOfAmmoSound;
}
void UArenaRangedWeaponEffects::SetOutOfAmmoSound(USoundCue* Value)
{
	OutOfAmmoSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetReloadSound()
{
	return ReloadSound;
}
void UArenaRangedWeaponEffects::SetReloadSound(USoundCue* Value)
{
	ReloadSound = Value;
}

USoundCue* UArenaRangedWeaponEffects::GetMeleeSound()
{
	return MeleeSound;
}
void UArenaRangedWeaponEffects::SetMeleeSound(USoundCue* Value)
{
	MeleeSound = Value;
}

//////////////////////////////////////// Animations ////////////////////////////////////////

bool UArenaRangedWeaponEffects::IsLoopedFireAnim()
{
	return LoopedFireAnim;
}
void UArenaRangedWeaponEffects::SetLoopedFireAnim(bool Value)
{
	LoopedFireAnim = Value;
}

bool UArenaRangedWeaponEffects::IsPlayingFireAnim()
{
	return PlayingFireAnim;
}
void UArenaRangedWeaponEffects::SetPlayingFireAnim(bool Value)
{
	PlayingFireAnim = Value;
}

UAnimMontage* UArenaRangedWeaponEffects::GetReloadAnim()
{
	return ReloadAnim;
}
void UArenaRangedWeaponEffects::SetReloadAnim(UAnimMontage* Value)
{
	ReloadAnim = Value;
}

UAnimMontage* UArenaRangedWeaponEffects::GetMeleeAnim()
{
	return MeleeAnim;
}
void UArenaRangedWeaponEffects::SetMeleeAnim(UAnimMontage* Value)
{
	MeleeAnim = Value;
}

UAnimMontage* UArenaRangedWeaponEffects::GetFireAnim()
{
	return FireAnim;
}
void UArenaRangedWeaponEffects::SetFireAnim(UAnimMontage* Value)
{
	FireAnim = Value;
}