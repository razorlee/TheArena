// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterAttributes.h"


// Sets default values for this component's properties
UArenaCharacterAttributes::UArenaCharacterAttributes(const FObjectInitializer& ObjectInitializer)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UArenaCharacterAttributes::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<AArenaCharacter>(GetOwner());

	MaxHealth = ResourcesConfig.Health;
	MaxStamina = ResourcesConfig.Stamina;
	MaxEnergy = ResourcesConfig.Energy;
	
}

void UArenaCharacterAttributes::Reset(AArenaCharacter* Owner)
{
	ResourcesConfig.Health = GetMaxHealth();
	ResourcesConfig.Energy = GetMaxEnergy();
	ResourcesConfig.Stamina = GetMaxStamina();
}

void UArenaCharacterAttributes::Regenerate(float DeltaSeconds)
{
	if (ResourcesConfig.Energy < GetMaxEnergy())
	{
		ResourcesConfig.Energy += StatsConfig.EnergyRegen * DeltaSeconds;
		if (ResourcesConfig.Energy > GetMaxEnergy())
		{
			ResourcesConfig.Energy = this->GetMaxEnergy();
		}
	}

	if (ResourcesConfig.Stamina < GetMaxStamina() && Owner->GetPlayerState()->GetPlayerState() != EPlayerState::Running)
	{
		ResourcesConfig.Stamina += StatsConfig.StaminaRegen * DeltaSeconds;
		if (ResourcesConfig.Stamina > GetMaxStamina())
		{
			ResourcesConfig.Stamina = GetMaxStamina();
		}
	}
}

int32 UArenaCharacterAttributes::GetMaxHealth() const
{
	return MaxHealth;
}

float UArenaCharacterAttributes::GetCurrentHealth() const
{
	return ResourcesConfig.Health;
}

void UArenaCharacterAttributes::SetCurrentHealth(float Value)
{
	ResourcesConfig.Health = Value;
}

int32 UArenaCharacterAttributes::GetMaxStamina() const
{
	return MaxStamina;
}

float UArenaCharacterAttributes::GetCurrentStamina() const
{
	return ResourcesConfig.Stamina;
}

void UArenaCharacterAttributes::SetCurrentStamina(float Value)
{
	ResourcesConfig.Stamina = Value;
}

int32 UArenaCharacterAttributes::GetMaxEnergy() const
{
	return MaxEnergy;
}

float UArenaCharacterAttributes::GetCurrentEnergy() const
{
	return ResourcesConfig.Energy;
}

void UArenaCharacterAttributes::SetCurrentEnergy(float Value)
{
	ResourcesConfig.Energy = Value;
}

int32 UArenaCharacterAttributes::GetMaxShields() const
{
	return GetClass()->GetDefaultObject<UArenaCharacterAttributes>()->ResourcesConfig.Shield;
}

float UArenaCharacterAttributes::GetCurrentShields() const
{
	return ResourcesConfig.Shield;
}

void UArenaCharacterAttributes::SetCurrentShields(float Value)
{
	ResourcesConfig.Shield = Value;
}

// Called every frame
void UArenaCharacterAttributes::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

