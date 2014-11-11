#include "TheArena.h"

AArenaImpactEffect::AArenaImpactEffect(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	ImpactLightComponentName = TEXT("ExplosionLight");

	PrimaryActorTick.bCanEverTick = true;

	ImpactLight = PCIP.CreateDefaultSubobject<UPointLightComponent>(this, ImpactLightComponentName);
	RootComponent = ImpactLight;
	ImpactLight->AttenuationRadius = 400.0;
	ImpactLight->Intensity = 500.0f;
	ImpactLight->bUseInverseSquaredFalloff = false;
	ImpactLight->LightColor = FColor(255, 185, 35);
	ImpactLight->CastShadows = false;
	ImpactLight->bVisible = true;

	ImpactLightFadeOut = 0.2f;
}

void AArenaImpactEffect::BeginPlay()
{
	Super::BeginPlay();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	// show particles
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	// play sound
	USoundBase* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (DefaultDecal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(DefaultDecal.DecalMaterial, FVector(DefaultDecal.DecalSize, DefaultDecal.DecalSize, 1.0f),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			DefaultDecal.LifeSpan);
	}
}

UParticleSystem* AArenaImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	
	UParticleSystem* ImpactFX = NULL;
	
	switch (SurfaceType)
	{
	
	case ARENA_SURFACE_Concrete:	ImpactFX = ConcreteFX; break;
	case ARENA_SURFACE_Dirt:		ImpactFX = DirtFX; break;
	case ARENA_SURFACE_Water:		ImpactFX = WaterFX; break;
	case ARENA_SURFACE_Metal:		ImpactFX = MetalFX; break;
	case ARENA_SURFACE_Wood:		ImpactFX = WoodFX; break;
	case ARENA_SURFACE_Grass:		ImpactFX = GrassFX; break;
	case ARENA_SURFACE_Glass:		ImpactFX = GlassFX; break;
	case ARENA_SURFACE_Flesh:		ImpactFX = FleshFX; break;
	case ARENA_SURFACE_Energy:		ImpactFX = EnergyFX; break;

	default:						ImpactFX = DefaultFX; break;
	}

	return ImpactFX;
	
}

USoundBase* AArenaImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	
	USoundBase* ImpactSound = NULL;

	switch (SurfaceType)
	{
	case ARENA_SURFACE_Concrete:	ImpactSound = ConcreteSound; break;
	case ARENA_SURFACE_Dirt:		ImpactSound = DirtSound; break;
	case ARENA_SURFACE_Water:		ImpactSound = WaterSound; break;
	case ARENA_SURFACE_Metal:		ImpactSound = MetalSound; break;
	case ARENA_SURFACE_Wood:		ImpactSound = WoodSound; break;
	case ARENA_SURFACE_Grass:		ImpactSound = GrassSound; break;
	case ARENA_SURFACE_Glass:		ImpactSound = GlassSound; break;
	case ARENA_SURFACE_Flesh:		ImpactSound = FleshSound; break;
	case ARENA_SURFACE_Energy:		ImpactSound = EnergySound; break;

	default:						ImpactSound = DefaultSound; break;
	}

	return ImpactSound;
	
}

void AArenaImpactEffect::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TimeAlive = GetWorld()->GetTimeSeconds() - CreationTime;
	const float TimeRemaining = FMath::Max(0.0f, ImpactLightFadeOut - TimeAlive);

	if (TimeRemaining > 0)
	{
		const float FadeAlpha = 1.0f - FMath::Square(TimeRemaining / ImpactLightFadeOut);

		UPointLightComponent* DefLight = Cast<UPointLightComponent>(GetClass()->GetDefaultSubobjectByName(ImpactLightComponentName));
		//ImpactLight->SetBrightness(DefLight->Intensity * FadeAlpha);
	}
	else
	{
		Destroy();
	}
}

