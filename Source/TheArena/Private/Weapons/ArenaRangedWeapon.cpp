// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeapon.h"

AArenaRangedWeapon::AArenaRangedWeapon(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponAttributes = ObjectInitializer.CreateDefaultSubobject<UArenaRangedWeaponAttributes>(this, TEXT("WeaponAttributes"));
	WeaponEffects = ObjectInitializer.CreateDefaultSubobject<UArenaRangedWeaponEffects>(this, TEXT("WeaponEffects"));
	WeaponState = ObjectInitializer.CreateDefaultSubobject<UArenaRangedWeaponState>(this, TEXT("WeaponState"));

	WeaponAttributes->SetNetAddressable();
	WeaponAttributes->SetIsReplicated(true);
	WeaponEffects->SetNetAddressable();
	WeaponEffects->SetIsReplicated(true);
	WeaponState->SetNetAddressable();
	WeaponState->SetIsReplicated(true);

	BurstCounter = 0;
	RecoilCounter = 0;
	IsRecoiling = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AArenaRangedWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsRecoiling)
	{
		HandleRecoil(DeltaSeconds);
	}
	else
	{
		FinishRecoil(DeltaSeconds);
	}
}

void AArenaRangedWeapon::Destroyed()
{
	Super::Destroyed();

	StopAttackFX();
}

/////////////////////////////////////////// Input handlers //////////////////////////////////////////

void AArenaRangedWeapon::StartAttack()
{
	if (Role < ROLE_Authority)
	{
		ServerStartAttack();
	}

	if (WeaponState->GetWeaponState() != EWeaponState::Firing)
	{
		WeaponState->SetWeaponState(EWeaponState::Firing);
		OnBurstStarted();
	}

}
void AArenaRangedWeapon::StopAttack()
{
	if (Role < ROLE_Authority)
	{
		ServerStopAttack();
	}

	if (WeaponState->GetWeaponState() == EWeaponState::Firing)
	{
		WeaponState->SetWeaponState(EWeaponState::Default);
		OnBurstFinished();
	}
}

void AArenaRangedWeapon::StartReload()
{
	if (ArenaWeaponCan::Reload(MyPawn, this))
	{
		if (Role == ROLE_Authority)
		{
			Reload();
		}
		else
		{
			ServerStartReload();
		}
	}
}
void AArenaRangedWeapon::StopReload_Implementation()
{
		if (WeaponState->GetWeaponState() == EWeaponState::Reloading)
		{
			WeaponState->SetWeaponState(EWeaponState::Default);
			StopWeaponAnimation(WeaponEffects->GetReloadAnim());
		}
}

////////////////////////////////////////// Action Functions /////////////////////////////////////////

void AArenaRangedWeapon::OnBurstStarted()
{
	const float GameTime = GetWorld()->GetTimeSeconds();

	if (WeaponAttributes->LastFireTime > 0 && WeaponAttributes->GetAttackSpeed() > 0.0f && WeaponAttributes->LastFireTime + WeaponAttributes->GetAttackSpeed() > GameTime)
	{
		if (WeaponAttributes->GetFireMode() == EFireMode::Burst)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleBurst, WeaponAttributes->LastFireTime + WeaponAttributes->GetAttackSpeed() - GameTime, false);
		}
		else
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleFiring, WeaponAttributes->LastFireTime + WeaponAttributes->GetAttackSpeed() - GameTime, false);
		}
	}
	else
	{
		if (WeaponAttributes->GetFireMode() == EFireMode::Burst)
		{
			GetWorldTimerManager().SetTimer(BurstFire, this, &AArenaRangedWeapon::HandleBurst, 0.05f, true);
		}
		else
		{
			HandleFiring();
		}
	}

}

void AArenaRangedWeapon::OnBurstFinished()
{
	if (WeaponState->GetWeaponState() == EWeaponState::Firing)
	{
		WeaponState->SetWeaponState(EWeaponState::Default);
	}
	if (WeaponAttributes->GetFireMode() == EFireMode::Burst)
	{
		if (BurstCounter > 2)
		{
			GetWorldTimerManager().ClearTimer(BurstFire);
			BurstCounter = 0;
		}
		StopAttackFX();
	}
	else
	{
		GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::HandleFiring);
		BurstCounter = 0;
		StopAttackFX();
	}
	
	IsRecoiling = false;
	bRefiring = false;
}

void AArenaRangedWeapon::HandleFiring()
{
	if (ArenaWeaponCan::Fire(MyPawn, this))
	{
		IsRecoiling = true;
		WeaponState->SetWeaponState(EWeaponState::Firing);

		if (GetNetMode() != NM_DedicatedServer)
		{
			PlayAttackFX();
		}
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();
			WeaponAttributes->CurrentClip--;
			BurstCounter++;
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// out of ammo
		if (WeaponAttributes->TotalAmmo == 0 && !bRefiring)
		{
			PlayWeaponSound(WeaponEffects->GetOutOfAmmoSound());
		}
		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// reload if possible
		if (WeaponAttributes->CurrentClip <= 0 && ArenaWeaponCan::Reload(MyPawn, this))
		{
			StopAttack();
			//GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::OnBurstFinished, 0.1f, false);
			StartReload();
		}

		bRefiring = (WeaponAttributes->GetFireMode() == EFireMode::Automatic && WeaponState->GetWeaponState() == EWeaponState::Firing && WeaponAttributes->GetAttackSpeed() > 0.0f);
		
		if (WeaponAttributes->GetFireMode() == EFireMode::Automatic && WeaponState->GetWeaponState() == EWeaponState::Firing)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleFiring, WeaponAttributes->GetAttackSpeed(), false);
		}
		else if (WeaponAttributes->GetFireMode() == EFireMode::Burst && BurstCounter > 2)
		{
			OnBurstFinished();
		}
		else
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::OnBurstFinished, 0.1f, false);
		}
	}
	WeaponAttributes->LastFireTime = GetWorld()->GetTimeSeconds();
}

void AArenaRangedWeapon::FireWeapon()
{
	for (int32 i = 0; i < WeaponAttributes->GetShotgunPellets(); i++)
	{
		FHitResult Hit = GetAdjustedAim();
		FVector ShootDir = Hit.ImpactPoint;
		FVector Origin = GetMuzzleLocation();

		const int32 RandomSeed = FMath::Rand();
		FRandomStream WeaponRandomStream(RandomSeed);
		const float CurrentSpread = GetCurrentSpread();
		const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

		ShootDir = WeaponRandomStream.VRandCone((ShootDir - Origin).GetSafeNormal(), ConeHalfAngle, ConeHalfAngle);

		ServerSpawnProjectile(Origin, ShootDir, Hit);
	}
}

void AArenaRangedWeapon::HandleBurst()
{
	GetWorldTimerManager().SetTimer(BurstFire, this, &AArenaRangedWeapon::HandleFiring, 0.05f, true);
}

void AArenaRangedWeapon::HandleRecoil(float DeltaSeconds)
{
	float Stability = 1 - (WeaponAttributes->GetStability() * 0.01);
	if (RecoilCounter < Stability * 15)
	{
		float Recoil = FMath::FInterpTo(0.0f, -Stability, DeltaSeconds, 100.0f);
		RecoilCounter += (Recoil * -1.0f);

		MyPawn->AddControllerPitchInput(Recoil);
	}
}

void AArenaRangedWeapon::FinishRecoil(float DeltaSeconds)
{
	if (RecoilCounter > 0)
	{
		float Recoil = FMath::FInterpTo(0.0f, RecoilCounter, DeltaSeconds, 2.5f);
		MyPawn->AddControllerPitchInput(Recoil);
		RecoilCounter -= Recoil;
	}
	else
	{
		RecoilCounter = 0;
	}
}

void AArenaRangedWeapon::Reload_Implementation()
{
	StopAttack();
	MyPawn->OnStopPeaking();
	WeaponState->SetWeaponState(EWeaponState::Reloading);
	float AnimDuration = PlayWeaponAnimation(WeaponEffects->GetReloadAnim(), WeaponAttributes->GetMotility()) * (1.0f / WeaponAttributes->GetMotility());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 3.0f;
	}

	GetWorldTimerManager().SetTimer(StopReload_Timer, this, &AArenaRangedWeapon::StopReload, AnimDuration, false);
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(Reload_Timer, this, &AArenaRangedWeapon::FinishReload, FMath::Max(0.1f, AnimDuration - 0.1f), false);
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(WeaponEffects->GetReloadSound());
	}
}

void AArenaRangedWeapon::FinishReload_Implementation()
{
	int32 ClipDelta = FMath::Min(WeaponAttributes->GetCapacity() - WeaponAttributes->CurrentClip, WeaponAttributes->TotalAmmo - WeaponAttributes->CurrentClip);

	if (ClipDelta > 0)
	{
		WeaponAttributes->CurrentClip += ClipDelta;
		WeaponAttributes->TotalAmmo -= ClipDelta;
	}
}

/////////////////////////////////////////// Aiming Helpers ////////////////////////////////////////// 

FVector AArenaRangedWeapon::GetCameraAim()
{
	FVector FinalAim = FVector::ZeroVector;

	FinalAim = Instigator->GetBaseAimRotation().Vector();
	return FinalAim;
}

FHitResult AArenaRangedWeapon::GetAdjustedAim()
{
	AArenaPlayerController* const PlayerController = Instigator ? Cast<AArenaPlayerController>(Instigator->Controller) : NULL;
	FHitResult Hit(ForceInit);
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		UCameraComponent* Camera = GetPawnOwner()->FollowCamera;
		FVector StartTrace = Camera->GetComponentLocation();
		FVector EndTrace = StartTrace + (Camera->GetForwardVector() * 100000.0f);

		static FName CameraFireTag = FName(TEXT("CameraTrace"));

		//GetWorld()->DebugDrawTraceTag = CameraFireTag;

		FCollisionQueryParams TraceParams(CameraFireTag, true, PlayerController);
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = true;


		GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_PROJECTILE, TraceParams);

		return Hit;// .ImpactPoint;//Camera->GetForwardVector();
	}
	return Hit;
}

FVector AArenaRangedWeapon::GetCameraDamageStartLocation(const FVector& AimDir)
{
	FVector OutStartTrace = FVector::ZeroVector;

	OutStartTrace = GetMuzzleLocation();
	return OutStartTrace;
}

FVector AArenaRangedWeapon::GetMuzzleLocation()
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AArenaRangedWeapon::GetMuzzleDirection()
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

float AArenaRangedWeapon::GetCurrentSpread() const
{
	float FinalSpread = WeaponAttributes->GetAccuracy();
	if (MyPawn)
	{
		if (MyPawn->GetPlayerState()->GetPlayerState() == EPlayerState::Crouching || MyPawn->GetPlayerState()->GetPlayerState() == EPlayerState::Covering)
		{
			if (WeaponState->GetTargetingState() == ETargetingState::Targeting || ETargetingState::Scoping)
			{
				FinalSpread = WeaponAttributes->GetAccuracy() * 0.25;
			}
			else
			{
				FinalSpread = WeaponAttributes->GetAccuracy() * 0.75;
			}
		}
		else
		{
			if (WeaponState->GetTargetingState() == ETargetingState::Targeting)
			{
				FinalSpread = WeaponAttributes->GetAccuracy() * 0.5;
			}
			else
			{
				FinalSpread = WeaponAttributes->GetAccuracy();
			}
		}
	}

	return FinalSpread;
}

FHitResult AArenaRangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace)
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info

	//GetWorld()->DebugDrawTraceTag = WeaponFireTag;
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_PROJECTILE, TraceParams);

	return Hit;
}

////////////////////////////////////////// Particle Effects /////////////////////////////////////////

void AArenaRangedWeapon::PlayAttackFX()
{
	if (Role == ROLE_Authority && WeaponState->GetWeaponState() != EWeaponState::Firing)
	{
		return;
	}

	if (WeaponEffects->GetMuzzleFX())
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (WeaponAttributes->GetFireMode() != EFireMode::Automatic || WeaponEffects->GetMuzzlePSC() == NULL)
		{
			 //Split screen requires we create 2 effects. One that we see and one that the other player sees.
			if (MyPawn && MyPawn->IsLocallyControlled())
			{
				AController* PlayerCon = MyPawn->GetController();
				if (PlayerCon != NULL)
				{
					Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					WeaponEffects->SetMuzzlePSC(UGameplayStatics::SpawnEmitterAttached(WeaponEffects->GetMuzzleFX(), Mesh3P, MuzzleAttachPoint));
					WeaponEffects->GetMuzzlePSC()->bOwnerNoSee = false;
					WeaponEffects->GetMuzzlePSC()->bOnlyOwnerSee = true;
				}
			}
			else
			{
				WeaponEffects->SetMuzzlePSC(UGameplayStatics::SpawnEmitterAttached(WeaponEffects->GetMuzzleFX(), UseWeaponMesh, MuzzleAttachPoint));
			}
		}
	}

	if (WeaponAttributes->GetFireMode() != EFireMode::Automatic || !WeaponEffects->IsPlayingFireAnim())
	{
		PlayWeaponAnimation(WeaponEffects->GetFireAnim());
		WeaponEffects->SetPlayingFireAnim(true);
	}

	if (WeaponAttributes->GetFireMode() == EFireMode::Automatic)
	{
		if (WeaponEffects->GetFireAC() == NULL)
		{
			WeaponEffects->SetFireAC(PlayWeaponSound(WeaponEffects->GetFireLoopSound()));
		}
	}
	else
	{
		PlayWeaponSound(WeaponEffects->GetFireStartSound());
	}

	AArenaPlayerController* PC = (MyPawn != NULL) ? Cast<AArenaPlayerController>(MyPawn->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		/*if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			PC->ClientPlayForceFeedback(FireForceFeedback, false, "Weapon");
		}*/
	}
}

void AArenaRangedWeapon::StopAttackFX()
{
	if (WeaponAttributes->GetFireMode() == EFireMode::Automatic)
	{
		if (WeaponEffects->GetMuzzlePSC() != NULL)
		{
			WeaponEffects->GetMuzzlePSC()->DeactivateSystem();
			WeaponEffects->SetMuzzlePSC(NULL);
		}
	}
	if (WeaponAttributes->GetFireMode() == EFireMode::Automatic && WeaponEffects->IsPlayingFireAnim())
	{
		StopWeaponAnimation(WeaponEffects->GetFireAnim());
		WeaponEffects->SetPlayingFireAnim(false);
	}

	if (WeaponEffects->GetFireAC())
	{
		WeaponEffects->GetFireAC()->FadeOut(0.1f, 0.0f);
		WeaponEffects->SetFireAC(NULL);

		PlayWeaponSound(WeaponEffects->GetFireFinishSound());
	}

}

///////////////////////////////////////// Weapon Components /////////////////////////////////////////

UArenaRangedWeaponState* AArenaRangedWeapon::GetWeaponState()
{
	return WeaponState;
}

UArenaRangedWeaponAttributes* AArenaRangedWeapon::GetWeaponAttributes()
{
	return WeaponAttributes;
}

UArenaRangedWeaponEffects* AArenaRangedWeapon::GetWeaponEffects()
{
	return WeaponEffects;
}

//////////////////////////////////////////// Replication ////////////////////////////////////////////

void AArenaRangedWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		PlayAttackFX();
	}
	else
	{
		StopAttackFX();
	}
}

void AArenaRangedWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, BurstCounter, COND_SkipOwner);
}

/////////////////////////////////////////////// Server //////////////////////////////////////////////

bool AArenaRangedWeapon::ServerStartAttack_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStartAttack_Implementation()
{
	StartAttack();
}

bool AArenaRangedWeapon::ServerStopAttack_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStopAttack_Implementation()
{
	StopAttack();
}

bool AArenaRangedWeapon::ServerHandleFiring_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (WeaponAttributes->CurrentClip > 0 && ArenaWeaponCan::Fire(MyPawn, this));

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		WeaponAttributes->CurrentClip--;
		BurstCounter++;
	}
}

bool AArenaRangedWeapon::ServerSpawnProjectile_Validate(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	return true;
}
void AArenaRangedWeapon::ServerSpawnProjectile_Implementation(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->SetPawnOwner(MyPawn);
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		Projectile->SetInitialSpeed(WeaponAttributes->GetVelocity());
		Projectile->InitVelocity(ShootDir);
		Projectile->SetHitResults(Hit);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

bool AArenaRangedWeapon::ServerStartReload_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStartReload_Implementation()
{
	Reload();
}

bool AArenaRangedWeapon::ServerStopReload_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStopReload_Implementation()
{
	StopReload();
}