#include "TheArena.h"

AArenaRangedWeapon::AArenaRangedWeapon(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	Mesh3P = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bPendingEquip = false;
	bIsEquipped = false;
	bPendingReload = false;
	bPendingMelee = false;
	bWantsToFire = false;
	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateInstigator = true;
	bNetUseOwnerRelevancy = true;
}

void AArenaRangedWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}

	DetachMeshFromPawn();
}

void AArenaRangedWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AArenaRangedWeapon::OnEquip()
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	float Duration = PlayWeaponAnimation(EquipAnim);
	if (Duration <= 0.0f)
	{
		// failsafe
		Duration = 0.5f;
	}
	EquipStartedTime = GetWorld()->GetTimeSeconds();
	EquipDuration = Duration;

	GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::OnEquipFinished, Duration, false);

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void AArenaRangedWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	// Determine the state so that the can reload checks will work
	DetermineWeaponState();

	if (MyPawn)
	{
		// try to reload empty clip
		if (MyPawn->IsLocallyControlled() &&
			CurrentAmmoInClip <= 0 &&
			CanReload())
		{
			StartReload();
		}
	}


}

void AArenaRangedWeapon::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::StopReload);
		GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::OnEquipFinished);
	}

	DetermineWeaponState();
}

void AArenaRangedWeapon::OnEnterInventory(AArenaCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AArenaRangedWeapon::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

void AArenaRangedWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
			Mesh3P->SetHiddenInGame(false);
			Mesh3P->AttachTo(PawnMesh3p, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachTo(UsePawnMesh, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}

void AArenaRangedWeapon::DetachMeshFromPawn()
{
	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
}


//////////////////////////////////////////////////////////////////////////
// Input

void AArenaRangedWeapon::StartFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AArenaRangedWeapon::StopFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AArenaRangedWeapon::StartReload(bool bFromReplication) //recall
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = WeaponConfig.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::StopReload, AnimDuration, false);
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AArenaRangedWeapon::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

void AArenaRangedWeapon::StartMelee(bool bFromReplication)
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartMelee();
	}

	if (bFromReplication || CanMelee())
	{
		bPendingMelee = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(MeleeAnim);
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
			PlayWeaponSound(MeleeSound);
		}
	}
}

void AArenaRangedWeapon::StopMelee()
{
	if (CurrentState == EWeaponState::Meleeing)
	{
		bPendingMelee = false;
		DetermineWeaponState();
		StopWeaponAnimation(MeleeAnim);
		HitActors.Empty();
	}
}

bool AArenaRangedWeapon::ServerStartFire_Validate()
{
	return true;
}

void AArenaRangedWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AArenaRangedWeapon::ServerStopFire_Validate()
{
	return true;
}

void AArenaRangedWeapon::ServerStopFire_Implementation()
{
	StopFire();
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

void AArenaRangedWeapon::ClientStartReload_Implementation()
{
	StartReload();
}

bool AArenaRangedWeapon::ServerStartMelee_Validate()
{
	return true;
}

void AArenaRangedWeapon::ServerStartMelee_Implementation()
{
	StartMelee();
}

bool AArenaRangedWeapon::ServerStopMelee_Validate()
{
	return true;
}

void AArenaRangedWeapon::ServerStopMelee_Implementation()
{
	StopMelee();
}

//////////////////////////////////////////////////////////////////////////
// Control

bool AArenaRangedWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}

bool AArenaRangedWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteClip());
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanReload == true) && (bGotAmmo == true) && (bStateOKToReload == true));
}

bool AArenaRangedWeapon::CanMelee() const
{
	bool bCanMelee = (!MyPawn || MyPawn->CanMelee());
	bool bStateOKToMelee = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return ((bCanMelee == true) && (bStateOKToMelee == true));
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AArenaRangedWeapon::GiveAmmo(int AddAmount)
{
	const int32 MissingAmmo = FMath::Max(0, WeaponConfig.MaxAmmo - CurrentAmmo);
	AddAmount = FMath::Min(AddAmount, MissingAmmo);
	CurrentAmmo += AddAmount;

	/*
	AShooterAIController* BotAI = MyPawn ? Cast<AShooterAIController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}*/

	// start reload if clip was empty
	if (GetCurrentAmmoInClip() <= 0 &&
		CanReload() &&
		MyPawn->GetWeapon() == this)
	{
		ClientStartReload();
	}
}

void AArenaRangedWeapon::UseAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmoInClip--;
	}

	if (!HasInfiniteAmmo() && !HasInfiniteClip())
	{
		CurrentAmmo--;
	}
	/*
	AArenaAIController* BotAI = MyPawn ? Cast<AArenaAIController>(MyPawn->GetController()) : NULL;
	AArenaPlayerController* PlayerController = MyPawn ? Cast<AArenaPlayerController>(MyPawn->GetController()) : NULL;
	if (BotAI)
	{
		BotAI->CheckAmmo(this);
	}
	else if (PlayerController)
	{
		AArenaPlayerState* PlayerState = Cast<AArenaPlayerState>(PlayerController->PlayerState);
		switch (GetAmmoType())
		{
		case EAmmoType::ERocket:
			PlayerState->AddRocketsFired(1);
			break;
		case EAmmoType::EBullet:
		default:
			PlayerState->AddBulletsFired(1);
			break;
		}
	}*/
}

void AArenaRangedWeapon::HandleFiring()
{
	if ((CurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();

			// update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(MyPawn->Controller);
			/*AShooterHUD* MyHUD = MyPC ? Cast<AShooterHUD>(MyPC->GetHUD()) : NULL;
			if (MyHUD)
			{
				MyHUD->NotifyOutOfAmmo();
			}*/
		}

		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// reload after firing last round
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleFiring, WeaponConfig.TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool AArenaRangedWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AArenaRangedWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// update ammo
		UseAmmo();

		// update firing FX on remote clients
		BurstCounter++;
	}
}

void AArenaRangedWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if (HasInfiniteClip())
	{
		ClipDelta = WeaponConfig.AmmoPerClip - CurrentAmmoInClip;
	}

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}

	if (HasInfiniteClip())
	{
		CurrentAmmo = FMath::Max(CurrentAmmoInClip, CurrentAmmo);
	}
}

void AArenaRangedWeapon::Melee()
{
	//Overlapping actors for each box spawned will be stored here
	TArray<struct FOverlapResult> OutOverlaps;
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
				UGameplayStatics::ApplyDamage(OutOverlaps[i].GetActor(), WeaponConfig.MeleeDamage, Instigator->GetController(), MyPawn->Controller, UDamageType::StaticClass());
			}
		}
		OutOverlaps.Empty();
	}
}

void AArenaRangedWeapon::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AArenaRangedWeapon::DetermineWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}
		else if ((bPendingReload == false) && (bWantsToFire == true) && (CanFire() == true))
		{
			NewState = EWeaponState::Firing;
		}
		else if ((bPendingMelee == true) && (CanMelee() == true))
		{
			NewState = EWeaponState::Meleeing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AArenaRangedWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f && LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(this, &AArenaRangedWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AArenaRangedWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::HandleFiring);
	bRefiring = false;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

UAudioComponent* AArenaRangedWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::PlaySoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float AArenaRangedWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AArenaRangedWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

FVector AArenaRangedWeapon::GetCameraAim() const
{
	FVector FinalAim = FVector::ZeroVector;

	FinalAim = Instigator->GetBaseAimRotation().Vector();
	return FinalAim;
}

FVector AArenaRangedWeapon::GetAdjustedAim() const
{
	FVector FinalAim = FVector::ZeroVector;
	
	FinalAim = Instigator->GetBaseAimRotation().Vector();
	return FinalAim;
}

FVector AArenaRangedWeapon::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	FVector OutStartTrace = FVector::ZeroVector;

	OutStartTrace = GetMuzzleLocation();
	return OutStartTrace;
}

FVector AArenaRangedWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AArenaRangedWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

FHitResult AArenaRangedWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void AArenaRangedWeapon::SetOwningPawn(AArenaCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AArenaRangedWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

void AArenaRangedWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void AArenaRangedWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

void AArenaRangedWeapon::OnRep_Melee()
{
	if (bPendingMelee)
	{
		StartMelee(true);
	}
	else
	{
		StopMelee();
	}
}

void AArenaRangedWeapon::SimulateWeaponFire()
{
	if (Role == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if (MuzzleFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			// Split screen requires we create 2 effects. One that we see and one that the other player sees.
			if ((MyPawn != NULL) && (MyPawn->IsLocallyControlled() == true))
			{
				AController* PlayerCon = MyPawn->GetController();
				if (PlayerCon != NULL)
				{
					Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = false;
				}
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
			}
		}
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	AArenaPlayerController* PC = (MyPawn != NULL) ? Cast<AArenaPlayerController>(MyPawn->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			PC->ClientPlayForceFeedback(FireForceFeedback, false, "Weapon");
		}
	}
}

void AArenaRangedWeapon::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC != NULL)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void AArenaRangedWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaRangedWeapon, MyPawn);

	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, CurrentAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, CurrentAmmoInClip, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, bPendingReload, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AArenaRangedWeapon, bPendingMelee, COND_SkipOwner);
}

USkeletalMeshComponent* AArenaRangedWeapon::GetWeaponMesh() const
{
	return Mesh3P;
}

class AArenaCharacter* AArenaRangedWeapon::GetPawnOwner() const
{
	return MyPawn;
}

bool AArenaRangedWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AArenaRangedWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState::Type AArenaRangedWeapon::GetCurrentState() const
{
	return CurrentState;
}

int32 AArenaRangedWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AArenaRangedWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AArenaRangedWeapon::GetAmmoPerClip() const
{
	return WeaponConfig.AmmoPerClip;
}

int32 AArenaRangedWeapon::GetMaxAmmo() const
{
	return WeaponConfig.MaxAmmo;
}

bool AArenaRangedWeapon::HasInfiniteAmmo() const
{
	const AArenaPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AArenaPlayerController>(MyPawn->Controller) : NULL;
	return WeaponConfig.bInfiniteAmmo || (MyPC && MyPC->HasInfiniteAmmo());
}

bool AArenaRangedWeapon::HasInfiniteClip() const
{
	const AArenaPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AArenaPlayerController>(MyPawn->Controller) : NULL;
	return WeaponConfig.bInfiniteClip || (MyPC && MyPC->HasInfiniteClip());
}

float AArenaRangedWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AArenaRangedWeapon::GetEquipDuration() const
{
	return EquipDuration;
}


