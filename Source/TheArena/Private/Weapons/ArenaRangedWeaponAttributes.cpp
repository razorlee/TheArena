// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeaponAttributes.h"

// Sets default values for this component's properties
UArenaRangedWeaponAttributes::UArenaRangedWeaponAttributes()
{
	SetIsReplicated(true);
	bReplicates = true;

	bWantsInitializeComponent = true;

	DamageType = UDamageType::StaticClass();

	ShotgunPellets = 1;
	HasScope = false;
	ZoomFOV = 90.0f;
	IsExplosive = false;
	ExplosionRadius = 0.0f;
}

void UArenaRangedWeaponAttributes::InitializeComponent()
{
	Super::InitializeComponent();

	TotalAmmo = GetCapacity() * 13;
	CurrentClip = GetCapacity();
	LastFireTime = 0;
}

FString UArenaRangedWeaponAttributes::GetDescription()
{
	return WeaponDescription;
}
void UArenaRangedWeaponAttributes::SetDescription(FString Value)
{
	WeaponDescription = Value;
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

bool UArenaRangedWeaponAttributes::GetHasScope()
{
	return HasScope;
}
void UArenaRangedWeaponAttributes::SetHasScope(bool Value)
{
	HasScope = Value;
}

float UArenaRangedWeaponAttributes::GetZoomFOV()
{
	return ZoomFOV;
}
void UArenaRangedWeaponAttributes::SetZoomFOV(float Value)
{
	ZoomFOV = Value;
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

void UArenaRangedWeaponAttributes::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UArenaRangedWeaponAttributes, TotalAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UArenaRangedWeaponAttributes, CurrentClip, COND_OwnerOnly);
}
