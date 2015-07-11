// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaRangedWeapon.h"

AArenaRangedWeapon::AArenaRangedWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

	RecoilCounter = 0;
	IsRecoiling = false;
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

////////////////////////////////////////// Input handlers //////////////////////////////////////////

void AArenaRangedWeapon::StartAttack()
{
	if (Role == ROLE_Authority)
	{
		OnBurstStarted();
	}
	else
	{
		ServerStartAttack();
	}
}
void AArenaRangedWeapon::StopAttack()
{
	if (Role == ROLE_Authority)
	{
		if (WeaponState->GetWeaponState() == EWeaponState::Firing)
		{
			WeaponState->SetWeaponState(EWeaponState::Default);
			OnBurstFinished();
		}
	}
	else
	{
		ServerStopAttack();
	}
}

void AArenaRangedWeapon::StartReload()
{
	if (Role == ROLE_Authority)
	{
		if (ArenaWeaponCan::Reload(MyPawn, this))
		{
			WeaponState->SetWeaponState(EWeaponState::Reloading);
			float AnimDuration = PlayWeaponAnimation(WeaponEffects->GetReloadAnim(), WeaponAttributes->GetMotility()) * (1.0f / WeaponAttributes->GetMotility());
			if (AnimDuration <= 0.0f)
			{
				AnimDuration = 3.0f;
			}

			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::StopReload, AnimDuration, false);
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::Reload, FMath::Max(0.1f, AnimDuration - 0.1f), false);
			}

			if (MyPawn && MyPawn->IsLocallyControlled())
			{
				PlayWeaponSound(WeaponEffects->GetReloadSound());
			}
		}
	}
	else
	{
		ServerStartReload();
	}
}
void AArenaRangedWeapon::StopReload()
{
	if (Role == ROLE_Authority)
	{
		if (WeaponState->GetWeaponState() == EWeaponState::Reloading)
		{
			WeaponState->SetWeaponState(EWeaponState::Default);
			StopWeaponAnimation(WeaponEffects->GetReloadAnim());
		}
	}
	else
	{
		ServerStopReload();
	}
}

void AArenaRangedWeapon::StartMelee()
{
	if (Role == ROLE_Authority)
	{
		float AnimDuration = PlayWeaponAnimation(WeaponEffects->GetMeleeAnim());
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = 0.3f;
		}

		GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::StopMelee, AnimDuration, false);
		if (Role == ROLE_Authority)
		{
			Melee();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(WeaponEffects->GetMeleeSound());
		}
	}
	else
	{
		//ServerStartMelee();
	}
}
void AArenaRangedWeapon::StopMelee()
{
	if (WeaponState->GetWeaponState() == EWeaponState::Meleeing)
	{
		StopWeaponAnimation(WeaponEffects->GetMeleeAnim());
		//HitActors.Empty();
	}
}

///////////////////////////////////////// Action Functions /////////////////////////////////////////

void AArenaRangedWeapon::OnBurstStarted()
{
	IsRecoiling = true;
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
			GetWorldTimerManager().SetTimer(BurstFire, this, &AArenaRangedWeapon::HandleFiring, 0.05f, true);
		}
		else
		{
			HandleFiring();
		}
	}

}

void AArenaRangedWeapon::OnBurstFinished()
{
	if (WeaponAttributes->GetFireMode() == EFireMode::Burst)
	{
		if (WeaponAttributes->BurstCounter > 2)
		{
			GetWorldTimerManager().ClearTimer(BurstFire);
			WeaponAttributes->BurstCounter = 0;
		}
		StopAttackFX();
	}
	else
	{
		GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::HandleFiring);
		WeaponAttributes->BurstCounter = 0;
		StopAttackFX();
	}
	
	IsRecoiling = false;
	bRefiring = false;
}

void AArenaRangedWeapon::HandleFiring()
{
	if (ArenaWeaponCan::Fire(MyPawn, this))
	{
		WeaponState->SetWeaponState(EWeaponState::Firing);

		if (GetNetMode() != NM_DedicatedServer)// && WeaponAttributes->BurstCounter < 1)
		{
			PlayAttackFX();
		}
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();
			WeaponAttributes->CurrentClip--;
			WeaponAttributes->BurstCounter++;
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// out of ammo
		if (WeaponAttributes->TotalAmmo == 0 && !bRefiring)
		{
			PlayWeaponSound(WeaponEffects->GetOutOfAmmoSound());
			AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(MyPawn->Controller);
		}
		// stop weapon fire FX, but stay in Firing state
		if (WeaponAttributes->BurstCounter > 0)
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
			OnBurstFinished();
			StartReload();
		}

		bRefiring = (WeaponAttributes->GetFireMode() == EFireMode::Automatic && WeaponState->GetWeaponState() == EWeaponState::Firing && WeaponAttributes->GetAttackSpeed() > 0.0f);
		
		if (WeaponAttributes->GetFireMode() == EFireMode::Automatic && WeaponState->GetWeaponState() == EWeaponState::Firing)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleFiring, WeaponAttributes->GetAttackSpeed(), false);
		}
		else if (WeaponAttributes->GetFireMode() == EFireMode::Burst && WeaponAttributes->BurstCounter > 2)
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

		SpawnProjectile(Origin, ShootDir, Hit);
		//CurrentFiringSpread = FMath::Min(ProjectileConfig.FiringSpreadMax, CurrentFiringSpread + ProjectileConfig.FiringSpreadIncrement);
	}
}

void AArenaRangedWeapon::SpawnProjectile(FVector Origin, FVector ShootDir, FHitResult Hit)
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

void AArenaRangedWeapon::HandleBurst()
{
	GetWorldTimerManager().SetTimer(BurstFire, this, &AArenaRangedWeapon::HandleFiring, 0.05f, true);
}

void AArenaRangedWeapon::HandleRecoil(float DeltaSeconds)
{
	float Stability = 1 - (WeaponAttributes->GetStability() * 0.01);
	if (RecoilCounter < Stability * 7)
	{
		RecoilCounter += Stability;

		float Recoil = FMath::FInterpTo(0.0f, -Stability, DeltaSeconds, 100.0f);

		MyPawn->AddControllerPitchInput(Recoil);
	}
}

void AArenaRangedWeapon::FinishRecoil(float DeltaSeconds)
{
	if (RecoilCounter > 0)
	{
		float Recoil = FMath::FInterpTo(0.0f, RecoilCounter, DeltaSeconds, 5.0f);
		MyPawn->AddControllerPitchInput(Recoil);
		RecoilCounter -= Recoil;
	}
	else
	{
		RecoilCounter = 0;
	}
}

void AArenaRangedWeapon::Reload()
{
	int32 ClipDelta = FMath::Min(WeaponAttributes->GetCapacity() - WeaponAttributes->CurrentClip, WeaponAttributes->TotalAmmo - WeaponAttributes->CurrentClip);

	if (ClipDelta > 0)
	{
		WeaponAttributes->CurrentClip += ClipDelta;
		WeaponAttributes->TotalAmmo -= ClipDelta;
	}
}

void AArenaRangedWeapon::Melee()
{
	//Overlapping actors for each box spawned will be stored here
	TArray<struct FOverlapResult> OutOverlaps;
	TArray<AActor*> HitActors;
	//The initial rotation of our box is the same as our character rotation
	FQuat Rotation = Instigator->GetTransform().GetRotation();
	FVector Start = Instigator->GetTransform().GetLocation() + Rotation.Rotator().Vector() * 100.0f;

	FCollisionShape CollisionHitShape;
	FCollisionQueryParams CollisionParams;
	//We do not want to store the instigator character in the array, so ignore it's collision
	CollisionParams.AddIgnoredActor(Instigator);

	//Set the channels that will respond to the collision
	FCollisionObjectQueryParams CollisionObjectTypes;
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	//Create the box and get the overlapping actors
	CollisionHitShape = FCollisionShape::MakeBox(FVector(60.0f, 60.0f, 0.5f));
	//CollisionHitShape = FCollisionShape::Cone
	GetWorld()->OverlapMulti(OutOverlaps, Start, Rotation, CollisionHitShape, CollisionParams, CollisionObjectTypes);

	//Process all hit actors
	for (int i = 0; i < OutOverlaps.Num(); ++i)
	{
		//We process each actor only once per Attack execution
		if (OutOverlaps[i].GetActor() && !HitActors.Contains(OutOverlaps[i].GetActor()))
		{
			//Process the actor to deal damage
			//CurrentWeapon->Melee(OutOverlaps[i].GetActor(), HitActors);
			//ServerMeleeAttack(CurrentWeapon, OutOverlaps[i].GetActor(), HitActors);


			if (!OutOverlaps[i].GetActor() || HitActors.Contains(OutOverlaps[i].GetActor()))
			{
				return;
			}
			//Add this actor to the array because we are spawning one box per tick and we don't want to hit the same actor twice during the same attack animation
			HitActors.AddUnique(OutOverlaps[i].GetActor());
			FHitResult AttackHitResult;
			const FDamageEvent AttackDamageEvent;
			AArenaCharacter* GameCharacter = Cast<AArenaCharacter>(OutOverlaps[i].GetActor());

			if (GameCharacter)
			{
				//OutOverlaps[i].GetActor()->TakeDamage(WeaponConfig.MeleeDamage, AttackDamageEvent, Instigator->GetController(), MyPawn->Controller);
				UGameplayStatics::ApplyDamage(OutOverlaps[i].GetActor(), 200.0f, Instigator->GetController(), MyPawn->Controller, UDamageType::StaticClass());
			}
		}
		OutOverlaps.Empty();
	}
}

///////////////////////////////////////// Aiming Helpers /////////////////////////////////////////

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
		FVector EndTrace = StartTrace + (Camera->GetForwardVector() * 10000.0f);

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
		if (WeaponState->GetTargetingState() == ETargetingState::Targeting || ETargetingState::Scoping)
		{
			FinalSpread = WeaponAttributes->GetAccuracy() * 0.5;
		}
		else
		{
			FinalSpread = WeaponAttributes->GetAccuracy();
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

///////////////////////////////////////// Particle Effects /////////////////////////////////////////

void AArenaRangedWeapon::PlayAttackFX()
{
	if (Role == ROLE_Authority && WeaponState->GetWeaponState() != EWeaponState::Firing)
	{
		return;
	}

	if (WeaponEffects->GetMuzzleFX())
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (WeaponAttributes->BurstCounter < 1 || WeaponEffects->GetMuzzlePSC() == NULL)
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
	if (WeaponEffects->GetMuzzlePSC() != NULL)
	{
		WeaponEffects->GetMuzzlePSC()->DeactivateSystem();
		WeaponEffects->SetMuzzlePSC(NULL);
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

////////////////////////////////////////////// Server //////////////////////////////////////////////

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
		WeaponAttributes->BurstCounter++;
	}
}

bool AArenaRangedWeapon::ServerStartReload_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AArenaRangedWeapon::ServerStopReload_Validate()
{
	return true;
}
void AArenaRangedWeapon::ServerStopReload_Implementation()
{
	StopReload();
}