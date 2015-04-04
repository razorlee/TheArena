#include "TheArena.h"

AArenaRangedWeapon_Projectile::AArenaRangedWeapon_Projectile(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	CurrentFiringSpread = 0.0f;
}

void AArenaRangedWeapon_Projectile::FireWeapon()
{
	FHitResult Hit = GetAdjustedAim();
	FVector ShootDir = Hit.ImpactPoint;
	FVector Origin = GetMuzzleLocation();
	const float ProjectileAdjustRange = 10000.0f;

	for (int32 i = 0; i < ProjectileConfig.BulletsPerRound; i++)
	{
		const int32 RandomSeed = FMath::Rand();
		FRandomStream WeaponRandomStream(RandomSeed);
		const float CurrentSpread = GetCurrentSpread();
		const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
		
		ShootDir = WeaponRandomStream.VRandCone((ShootDir - Origin).GetSafeNormal(), ConeHalfAngle, ConeHalfAngle);
		//ShootDir = (ShootDir - Origin).GetSafeNormal();

		ServerFireProjectile(Origin, ShootDir, Hit);
		CurrentFiringSpread = FMath::Min(ProjectileConfig.FiringSpreadMax, CurrentFiringSpread + ProjectileConfig.FiringSpreadIncrement);
	}
}

bool AArenaRangedWeapon_Projectile::ServerFireProjectile_Validate(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	return true;
}

void AArenaRangedWeapon_Projectile::ServerFireProjectile_Implementation(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetPawnOwner(MyPawn);
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);
		Projectile->SetHitResults(Hit);
			
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

float AArenaRangedWeapon_Projectile::GetCurrentSpread() const
{
	float FinalSpread = ProjectileConfig.WeaponSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= ProjectileConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

void AArenaRangedWeapon_Projectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}

