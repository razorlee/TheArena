// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterAttributes.h"


// Sets default values for this component's properties
UArenaCharacterAttributes::UArenaCharacterAttributes(const FObjectInitializer& ObjectInitializer)
{
	SetIsReplicated(true);
	bReplicates = true;

	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UArenaCharacterAttributes::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<AArenaCharacter>(GetOwner());

	CurrentHealth = ResourcesConfig.Health;
	CurrentEnergy = ResourcesConfig.Stamina;
	CurrentStamina = ResourcesConfig.Energy;
	CurrentShield = ResourcesConfig.Shield;
	
}

void UArenaCharacterAttributes::Reset(AArenaCharacter* Owner)
{
	CurrentHealth = ResourcesConfig.Health;
	CurrentEnergy = ResourcesConfig.Energy;
	CurrentStamina = ResourcesConfig.Stamina;
}

void UArenaCharacterAttributes::Regenerate(float DeltaSeconds)
{
	if (CurrentEnergy < ResourcesConfig.Energy)
	{
		CurrentEnergy += StatsConfig.EnergyRegen * DeltaSeconds;
		if (CurrentEnergy > ResourcesConfig.Energy)
		{
			CurrentEnergy = this->GetMaxEnergy();
		}
	}
	if (CurrentEnergy > ResourcesConfig.Energy)
	{
		CurrentEnergy = ResourcesConfig.Energy;
	}
	if (CurrentEnergy < 0.0)
	{
		CurrentEnergy = 0;
	}

	if (CurrentHealth > ResourcesConfig.Health)
	{
		CurrentHealth = ResourcesConfig.Health;
	}
	if (CurrentHealth < 0.0)
	{
		CurrentHealth = 0;
	}

	if (CurrentStamina < ResourcesConfig.Stamina)
	{
		CurrentStamina += StatsConfig.StaminaRegen * DeltaSeconds;
		if (CurrentStamina > ResourcesConfig.Stamina)
		{
			CurrentStamina = GetMaxStamina();
		}
	}
	if (CurrentStamina > ResourcesConfig.Stamina)
	{
		CurrentStamina = ResourcesConfig.Stamina;
	}
	if (CurrentStamina < 0.0)
	{
		CurrentStamina = 0;
	}
}

int32 UArenaCharacterAttributes::GetMaxHealth() const
{
	return ResourcesConfig.Health;
}

float UArenaCharacterAttributes::GetCurrentHealth() const
{
	return CurrentHealth;
}

void UArenaCharacterAttributes::SetCurrentHealth(float Value)
{
	CurrentHealth = Value;
}

int32 UArenaCharacterAttributes::GetMaxStamina() const
{
	return ResourcesConfig.Stamina;
}

float UArenaCharacterAttributes::GetCurrentStamina() const
{
	return CurrentStamina;
}

void UArenaCharacterAttributes::SetCurrentStamina(float Value)
{
	CurrentStamina = Value;
}

int32 UArenaCharacterAttributes::GetMaxEnergy() const
{
	return ResourcesConfig.Energy;
}

float UArenaCharacterAttributes::GetCurrentEnergy() const
{
	return CurrentEnergy;
}

void UArenaCharacterAttributes::SetCurrentEnergy(float Value)
{
	CurrentEnergy = Value;
}

int32 UArenaCharacterAttributes::GetMaxShields() const
{
	return ResourcesConfig.Health;
}

float UArenaCharacterAttributes::GetCurrentShields() const
{
	return CurrentShield;
}

void UArenaCharacterAttributes::SetCurrentShields(float Value)
{
	CurrentShield = Value;
}


float UArenaCharacterAttributes::GetSpeed() const
{
	return StatsConfig.Speed;
}

void UArenaCharacterAttributes::SetSpeed(float Value)
{
	StatsConfig.Speed = Value;
}


float UArenaCharacterAttributes::GetProtection() const
{
	return StatsConfig.Protection;
}

void UArenaCharacterAttributes::SetProtection(float Value)
{
	StatsConfig.Protection = Value;
}

// Called every frame
void UArenaCharacterAttributes::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UArenaCharacterAttributes::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UArenaCharacterAttributes, CurrentEnergy);
	DOREPLIFETIME(UArenaCharacterAttributes, CurrentHealth);
	DOREPLIFETIME(UArenaCharacterAttributes, CurrentStamina);
}

