// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaSniperRifle.h"

AArenaSniperRifle::AArenaSniperRifle(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentFiringSpread = 0.0f;
}

void AArenaSniperRifle::FireWeapon()
{
	FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();
	const float ProjectileAdjustRange = 10000.0f;

	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;

	// trace from camera to check what's under crosshair
	//const float ProjectileAdjustRange = 10000.0f;
	//const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	//const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;

	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	// and adjust directions to hit that actor
	if (Impact.bBlockingHit)
	{
		const FVector AdjustedDir = (Impact.ImpactPoint - Origin).SafeNormal();
		bool bWeaponPenetration = false; 

		const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
		if (DirectionDot < 0.0f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (DirectionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
			FVector MuzzleEndTrace = Origin;
			FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			Origin = Impact.ImpactPoint - ShootDir * 10.0f;
		}
		else
		{
			// adjust direction to hit
			ShootDir = AdjustedDir;
		}
	}

	ServerFireProjectile(Origin, ShootDir);
	CurrentFiringSpread = FMath::Min(ProjectileConfig.FiringSpreadMax, CurrentFiringSpread + ProjectileConfig.FiringSpreadIncrement);
}

bool AArenaSniperRifle::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AArenaSniperRifle::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetPawnOwner(MyPawn);
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

float AArenaSniperRifle::GetCurrentSpread() const
{
	float FinalSpread = ProjectileConfig.WeaponSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= ProjectileConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

void AArenaSniperRifle::ApplyWeaponConfig(FSniperWeaponData& Data)
{
	Data = ProjectileConfig;
}




