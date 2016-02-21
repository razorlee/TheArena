// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaUtility.h"

// Sets default values
AArenaUtility::AArenaUtility(const class FObjectInitializer& PCIP)
{
	Mesh3P = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("UtilityMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Channel = COLLISION_PROJECTILE;
	Active = false;
}

void AArenaUtility::Destroyed()
{
	Super::Destroyed();
}

// Called when the game starts or when spawned
void AArenaUtility::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaUtility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Active && (UtilityStats.ActivationType == EActivationType::Toggle || UtilityStats.ActivationType == EActivationType::Channel))
	{
		ConsumeEnergy(UtilityStats.ContinuationCost, DeltaTime);
		if (MyPawn->GetCharacterAttributes()->GetCurrentEnergy() <= 0.0f)
		{
			Deactivate();
			Active = false;
		}
	}
}

class AArenaCharacter* AArenaUtility::GetMyPawn() const
{
	return MyPawn;
}
void AArenaUtility::SetMyPawn(AArenaCharacter* Pawn)
{
	Instigator = Pawn;
	MyPawn = Pawn;
	SetOwner(Pawn);
}

FName AArenaUtility::GetUtilityName() const
{
	return UtilityName;
}

FString AArenaUtility::GetDescription()
{
	return UtilityDescription;
}
void AArenaUtility::SetDescription(FString Value)
{
	UtilityDescription = Value;
}

float AArenaUtility::GetActivationCost() const
{
	return UtilityStats.ActivationCost;
}
void AArenaUtility::SetActivationCost(float Value)
{
	UtilityStats.ActivationCost = Value;
}

void AArenaUtility::Equip()
{
	FName AttachPoint;
	if (UtilityStats.UtilityType == EUtilityType::Head)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainHeavyAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffHeavyAttachPoint();
	}
	else if (UtilityStats.UtilityType == EUtilityType::UpperBack)
	{
		AttachPoint = FName(TEXT("UpperBackUtility"));
	}
	else if (UtilityStats.UtilityType == EUtilityType::LowerBack)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (UtilityStats.UtilityType == EUtilityType::Wrist)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (UtilityStats.UtilityType == EUtilityType::Waist)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}

	if (MyPawn->IsLocallyControlled() == true)
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
	else
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
}
void AArenaUtility::UnEquip()
{
	if (Role == ROLE_Authority)
	{
		SetMyPawn(NULL);
	}

	Deactivate();
	Active = false;
	
	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
}

EUtilityType::Type AArenaUtility::GetUtilityType()
{
	return UtilityStats.UtilityType;
}

EActivationType::Type AArenaUtility::GetActivationType()
{
	return UtilityStats.ActivationType;
}

void AArenaUtility::Activate()
{
	if (Role == ROLE_Authority)
	{
		if (!Active)
		{
			ConsumeEnergy(UtilityStats.ActivationCost);
			PlayUtilityAnimation();
		}
		//Active = true;
		ActivateBP();
		// Do nothing here
	}
	else
	{
		ServerActivate();
	}
}
void AArenaUtility::Deactivate()
{
	if (Role == ROLE_Authority)
	{
		DeactivateBP();
		//Active = false;
		// Do nothing here
	}
	else
	{
		ServerDeactivate();
	}
}

void AArenaUtility::ConsumeEnergy(float DeltaSeconds, float Cost)
{
	MyPawn->GetCharacterAttributes()->SetCurrentEnergy(MyPawn->GetCharacterAttributes()->GetCurrentEnergy() - (Cost * DeltaSeconds));
}

void AArenaUtility::FireProjectile()
{
	FHitResult Hit = GetAdjustedAim();
	FVector ShootDir = Hit.ImpactPoint;
	FVector Origin = GetSocketLocation();

	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = 0.0f;
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	ShootDir = WeaponRandomStream.VRandCone((ShootDir - Origin).GetSafeNormal(), ConeHalfAngle, ConeHalfAngle);

	//SpawnTrailEffect(Hit.ImpactPoint); may not need this for utilities
	ServerSpawnProjectile(Origin, ShootDir, Hit);
	//Test(Origin, ShootDir, Hit);

}

FHitResult AArenaUtility::GetAdjustedAim()
{
	AArenaPlayerController* const PlayerController = Instigator ? Cast<AArenaPlayerController>(Instigator->Controller) : NULL;
	FHitResult Hit(ForceInit);
	// If we have a player controller use it for the aim
	if (PlayerController)
	{
		UCameraComponent* Camera = GetMyPawn()->FollowCamera;
		FVector StartTrace = Camera->GetComponentLocation();
		FVector EndTrace = StartTrace + (Camera->GetForwardVector() * 100000.0f);

		static FName CameraFireTag = FName(TEXT("CameraTrace"));

		//GetWorld()->DebugDrawTraceTag = CameraFireTag;

		FCollisionQueryParams TraceParams(CameraFireTag, true, PlayerController);
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = true;


		GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, Channel, TraceParams);

		return Hit;// .ImpactPoint;//Camera->GetForwardVector();
	}
	return Hit;
}

FVector AArenaUtility::GetSocketLocation()
{
	USkeletalMeshComponent* UseMesh = GetMyPawn()->GetPawnMesh();
	return UseMesh->GetSocketLocation("UtilityProjectile");
}

void AArenaUtility::PlayUtilityAnimation_Implementation()
{
	if (UtilityAnim)
	{
		MyPawn->PlayAnimMontage(UtilityAnim);
	}
}

/////////////////////////////////////// Server ///////////////////////////////////////

void AArenaUtility::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaUtility, MyPawn);
	DOREPLIFETIME(AArenaUtility, Active);
}

void AArenaUtility::OnRep_MyPawn()
{
	if (MyPawn)
	{
		SetMyPawn(MyPawn);
	}
	else
	{
		UnEquip();
	}
}

bool AArenaUtility::ServerActivate_Validate()
{
	return true;
}
void AArenaUtility::ServerActivate_Implementation()
{
	if (!Active)
	{
		ConsumeEnergy(UtilityStats.ActivationCost);
		PlayUtilityAnimation();
	}
	//Active = true;
	ActivateBP();
}

bool AArenaUtility::ServerDeactivate_Validate()
{
	return true;
}
void AArenaUtility::ServerDeactivate_Implementation()
{
	DeactivateBP();
	//Active = false;
}

bool AArenaUtility::ServerSpawnProjectile_Validate(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	return true;
}
void AArenaUtility::ServerSpawnProjectile_Implementation(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		FTransform SpawnTM(ShootDir.Rotation(), Origin);
		AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetPawnOwner(MyPawn);
			Projectile->Instigator = Instigator;
			Projectile->SetOwner(this);
			Projectile->SetInitialSpeed(UtilityStats.Velocity);
			Projectile->SetCollisionChannel(Channel);
			Projectile->InitVelocity(ShootDir);
			Projectile->SetDamage(UtilityStats.Damage);
			Projectile->SetIsExplosive(UtilityStats.IsExplosive);
			Projectile->SetExplosionRadius(UtilityStats.ExplosionRadius);
			Projectile->SetIsAffectByVelocity(true);
			Projectile->SetHitResults(Hit);

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			Projectile->StartTimer();
		}
	}
	else
	{
		ShootDir = MyPawn->FollowCamera->GetForwardVector();
		FTransform SpawnTM(ShootDir.Rotation(), Origin); //new function
		AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetPawnOwner(MyPawn);
			Projectile->Instigator = Instigator;
			Projectile->SetOwner(this);
			Projectile->SetInitialSpeed(UtilityStats.Velocity);
			Projectile->SetCollisionChannel(Channel);
			Projectile->InitVelocity(ShootDir);
			Projectile->SetDamage(UtilityStats.Damage);
			Projectile->SetIsExplosive(UtilityStats.IsExplosive);
			Projectile->SetExplosionRadius(UtilityStats.ExplosionRadius);
			Projectile->SetIsAffectByVelocity(true);

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			Projectile->StartTimer();
		}
	}
}

void AArenaUtility::Test(FVector Origin, FVector ShootDir, FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		FTransform SpawnTM(ShootDir.Rotation(), Origin);
		AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetPawnOwner(MyPawn);
			Projectile->Instigator = Instigator;
			Projectile->SetOwner(this);
			Projectile->SetInitialSpeed(UtilityStats.Velocity);
			Projectile->SetCollisionChannel(Channel);
			Projectile->InitVelocity(ShootDir);
			Projectile->SetDamage(UtilityStats.Damage);
			Projectile->SetIsExplosive(UtilityStats.IsExplosive);
			Projectile->SetExplosionRadius(UtilityStats.ExplosionRadius);
			Projectile->SetIsAffectByVelocity(true);
			Projectile->SetHitResults(Hit);

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			Projectile->StartTimer();
		}
	}
	else
	{
		ShootDir = MyPawn->FollowCamera->GetForwardVector();
		FTransform SpawnTM(ShootDir.Rotation(), Origin); //new function
		AArenaProjectile* Projectile = Cast<AArenaProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetPawnOwner(MyPawn);
			Projectile->Instigator = Instigator;
			Projectile->SetOwner(this);
			Projectile->SetInitialSpeed(UtilityStats.Velocity);
			Projectile->SetCollisionChannel(Channel);
			Projectile->InitVelocity(ShootDir);
			Projectile->SetDamage(UtilityStats.Damage);
			Projectile->SetIsExplosive(UtilityStats.IsExplosive);
			Projectile->SetExplosionRadius(UtilityStats.ExplosionRadius);
			Projectile->SetIsAffectByVelocity(true);

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			Projectile->StartTimer();
		}
	}
}