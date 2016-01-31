#include "TheArena.h"

AArenaProjectile::AArenaProjectile(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	CollisionComp = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(COLLISION_PROJECTILEPEN, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	ParticleComp = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = false;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->AttachParent = RootComponent;

	MovementComp = PCIP.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 100000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	//bReplicateInstigator = true;
	bReplicateMovement = true;

	Damage = 0.0f;
	IsExplosive = false;
	ExplosionRadius = 0.0f;
	IsAffectByVelocity = false;
}

void AArenaProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MovementComp->OnProjectileStop.AddDynamic(this, &AArenaProjectile::OnImpact);
	CollisionComp->MoveIgnoreActors.Add(Instigator);

	//SetLifeSpan(6.0f);
	MyController = GetInstigatorController();
}

//////////////////////////////////////////// Projectile Functions ////////////////////////////////////////////

void AArenaProjectile::InitVelocity(FVector& ShootDirection)
{
	if (MovementComp)
	{
		MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
	}
}

void AArenaProjectile::SetCollisionChannel(ECollisionChannel Value)
{
	if (CollisionComp)
	{
		CollisionComp->SetCollisionObjectType(Value);
		CollisionComp->SetCollisionResponseToChannel(Value, ECR_Ignore);
	}
}

void AArenaProjectile::OnImpact(const FHitResult& HitResult)
{
	if (Role == ROLE_Authority && !bExploded && !MovementComp->bShouldBounce)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

void AArenaProjectile::Explode(const FHitResult& Impact)
{
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	// effects and damage origin shouldn't be placed inside mesh at impact point
	FVector NudgedImpactLocation;
	if (Impact.IsValidBlockingHit() && !MovementComp->bShouldBounce)
	{
		NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
	}
	else
	{
		NudgedImpactLocation = this->GetActorLocation() + this->GetActorLocation().Normalize() * 10.0f;
	}
	if (IsExplosive)
	{
		if (Damage != 0 && ExplosionRadius > 0 && UDamageType::StaticClass())
		{
			UGameplayStatics::ApplyRadialDamage(this, Damage, NudgedImpactLocation, ExplosionRadius, UDamageType::StaticClass(), TArray<AActor*>(), this, MyController.Get());
		}
	}
	else
	{
		if (Damage != 0 && UDamageType::StaticClass())
		{
			FString critical = "head";
			if (Impact.BoneName.ToString() == critical)
			{
				Damage = (Damage * 2) * (IsAffectByVelocity ? (StartTime / StopTimer()) : 1.0f);
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%f Damage"), Damage));
				UGameplayStatics::ApplyPointDamage(Impact.GetActor(), Damage, Impact.ImpactPoint, Impact, MyPawn->Controller, this, UDamageType::StaticClass());
			}
			else
			{
				Damage = (Damage) * (IsAffectByVelocity ? (StartTime / StopTimer()) : 1.0f);
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%f Damage"), Damage));
				UGameplayStatics::ApplyPointDamage(Impact.GetActor(), Damage, Impact.ImpactPoint, Impact, MyPawn->Controller, this, UDamageType::StaticClass());
			}
		}
	}
	SpawnImpactEffects(Impact);

	ExplodeNotify.bExploded = true;
	ExplodeNotify.Hit = Impact;
}

void AArenaProjectile::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate)// && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;
		// trace again to find component lost during replication

		const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
		const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
		FHitResult Hit = ProjectileTrace(StartTrace, EndTrace);
		UseImpact = Hit;

		AArenaImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AArenaImpactEffect>(ImpactTemplate, Impact.ImpactPoint, Impact.ImpactNormal.Rotation());
		if (EffectActor)
		{
			FVector NudgedImpactLocation;
			if (Impact.IsValidBlockingHit() && !MovementComp->bShouldBounce)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Material: %s"), UseImpact.PhysMaterial));
				EffectActor->SurfaceHit = UseImpact;
				UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
			}
			else
			{
				UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(this->GetActorLocation().Rotation(), this->GetActorLocation()));
			}

		}
	}
}

FHitResult AArenaProjectile::ProjectileTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info

	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_PROJECTILE, TraceParams);

	return Hit;
}

void AArenaProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	// give clients some time to show explosion
	SetLifeSpan(0.1f);
}

///////////////////////////////////////////// Setting Essentials /////////////////////////////////////////////

void AArenaProjectile::SetInitialSpeed(float Speed)
{
	MovementComp->InitialSpeed = Speed;
}

void AArenaProjectile::SetPawnOwner(class AArenaCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		MyPawn = NewOwner;
	}
}

void AArenaProjectile::SetHitResults(const FHitResult& Impact)
{
	HitResults = Impact;
}

void AArenaProjectile::SetDamage(float Value)
{
	Damage = Value;
}

void AArenaProjectile::SetIsExplosive(bool Value)
{
	IsExplosive = Value;
}

void AArenaProjectile::SetExplosionRadius(float Value)
{
	ExplosionRadius = Value;
}

void AArenaProjectile::SetIsAffectByVelocity(bool Value)
{
	IsAffectByVelocity = Value;
}

void AArenaProjectile::StartTimer()
{
	 StartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
}

float AArenaProjectile::StopTimer()
{
	StopTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	return StopTime;
}

void AArenaProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AArenaProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaProjectile, MyPawn);
	DOREPLIFETIME(AArenaProjectile, ExplodeNotify);
}

///////////////////////////////////////// Replication & Networking /////////////////////////////////////////

void AArenaProjectile::OnRep_Exploded()
{
	Explode(ExplodeNotify.Hit);
}