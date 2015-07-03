// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeaponAttributes.h"

// Sets default values for this component's properties
UArenaRangedWeaponAttributes::UArenaRangedWeaponAttributes()
{
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	DamageType = UDamageType::StaticClass();

	ShotgunPellets = 1;
	IsExplosive = false;
	ExplosionRadius = 0.0f;
}

// Called when the game starts
void UArenaRangedWeaponAttributes::BeginPlay()
{
	Super::BeginPlay();

	TotalAmmo = GetCapacity() * 13;
	CurrentClip = GetCapacity();
	BurstCounter = 0;
	LastFireTime = 0;
}

// Called every frame
void UArenaRangedWeaponAttributes::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

int32 UArenaRangedWeaponAttributes::GetDamage()
{
	return WeaponAttributes.Damage;
}
void UArenaRangedWeaponAttributes::SetDamage(int32 Value)
{
	WeaponAttributes.Damage = Value;
}

float UArenaRangedWeaponAttributes::GetStability()
{
	return WeaponAttributes.Stability;
}
void UArenaRangedWeaponAttributes::SetStability(float Value)
{
	WeaponAttributes.Stability = Value;
}

float UArenaRangedWeaponAttributes::GetMotility()
{
	return WeaponAttributes.Motility;
}
void UArenaRangedWeaponAttributes::SetMotility(float Value)
{
	WeaponAttributes.Motility = Value;
}

float UArenaRangedWeaponAttributes::GetVelocity()
{
	return WeaponAttributes.Velocity;
}
void UArenaRangedWeaponAttributes::SetVelocity(float Value)
{
	WeaponAttributes.Velocity = Value;
}

float UArenaRangedWeaponAttributes::GetAccuracy()
{
	return WeaponAttributes.Accuracy;
}
void UArenaRangedWeaponAttributes::SetAccuracy(float Value)
{
	WeaponAttributes.Accuracy = Value;
}

float UArenaRangedWeaponAttributes::GetAttackSpeed()
{
	return WeaponAttributes.AttackSpeed;
}
void UArenaRangedWeaponAttributes::SetAttackSpeed(float Value)
{
	WeaponAttributes.AttackSpeed = Value;
}

float UArenaRangedWeaponAttributes::GetMobility()
{
	return WeaponAttributes.Mobility;
}
void UArenaRangedWeaponAttributes::SetMobility(float Value)
{
	WeaponAttributes.Mobility = Value;
}

int32 UArenaRangedWeaponAttributes::GetCapacity()
{
	return WeaponAttributes.Capacity;
}
void UArenaRangedWeaponAttributes::SetCapacity(int32 Value)
{
	WeaponAttributes.Capacity = Value;
}

EFireMode::Type UArenaRangedWeaponAttributes::GetFireMode()
{
	return FireMode;
}
void UArenaRangedWeaponAttributes::SetFireMode(EFireMode::Type NewMode)
{
	FireMode = NewMode;
}

int32 UArenaRangedWeaponAttributes::GetShotgunPellets()
{
	return ShotgunPellets;
}
void UArenaRangedWeaponAttributes::SetShotgunPellets(int32 Value)
{
	ShotgunPellets = Value;
}

bool UArenaRangedWeaponAttributes::GetIsExplosive()
{
	return IsExplosive;
}
void UArenaRangedWeaponAttributes::SetIsExplosive(bool Value)
{
	IsExplosive = Value;
}

int32 UArenaRangedWeaponAttributes::GetExplosionRadius()
{
	return ExplosionRadius;
}
void UArenaRangedWeaponAttributes::SetExplosionRadius(int32 Value)
{
	ExplosionRadius = Value;
}

TSubclassOf<UDamageType> UArenaRangedWeaponAttributes::GetDamageType()
{
	return DamageType;
}