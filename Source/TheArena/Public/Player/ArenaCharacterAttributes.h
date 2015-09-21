// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterAttributes.generated.h"

USTRUCT(BlueprintType)
struct FPlayerStats
{
	GENERATED_USTRUCT_BODY()

	/** Rate of health regeneration */
	UPROPERTY(EditAnywhere, Category = Stats)
	float HealthRegen;

	/**Rate of stamina regeneration */
	UPROPERTY(EditAnywhere, Category = Stats)
	float StaminaRegen;

	/** Rate of energy regeneration */
	UPROPERTY(EditAnywhere, Category = Stats)
	float EnergyRegen;

	/** Rate of shield regeneration */
	UPROPERTY(EditAnywhere, Category = Stats)
	float ShieldRegen;

	/** Increases the amount of damage mitigated */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Protection;

	/** Reduces stamina cunsumption and increase stamina regeneration */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Mobility;

	/** Decreases movement speed reduction */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Speed;

	/** Increases the players rate of recovery from negative effects */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Recovery;

	///** defaults */
	FPlayerStats()
	{
		HealthRegen = 5.0f;
		StaminaRegen = 5.0f;
		EnergyRegen = 5.0f;
		ShieldRegen = 0.0f;
		Protection = 0.0f;
		Mobility = 0.0f;
		Speed = 0.0f;
		Recovery = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FPlayerResources
{
	GENERATED_USTRUCT_BODY()

	/** Current health of the Pawn */
	UPROPERTY(EditAnywhere, Category = Resources)
	float Health;

	/** Current Stamina of the Pawn */
	UPROPERTY(EditAnywhere, Category = Resources)
	float Stamina;

	/** Current Energy of the Pawn */
	UPROPERTY(EditAnywhere, Category = Resources)
	float Energy;

	/** Current Shield of the Pawn */
	UPROPERTY(EditAnywhere, Category = Resources)
	float Shield;

	/** The percentage of health when low health effects should start */
	UPROPERTY(EditAnywhere, Category = Resources)
	float LowHealthPercentage;

	/** The percentage of stamina when low stamina effects should start */
	UPROPERTY(EditAnywhere, Category = Resources)
	float LowStaminaPercentage;

	/** The percentage of energy when low energy effects should start */
	UPROPERTY(EditAnywhere, Category = Resources)
	float LowEnergyPercentage;

	/** The percentage of shield when low shield effects should start */
	UPROPERTY(EditAnywhere, Category = Resources)
	float LowShieldPercentage;

	/** defaults */
	FPlayerResources()
	{
		Health = 1000.0f;
		Stamina = 1000.0f;
		Energy = 1000.0f;
		Shield = 0.0f;
		LowHealthPercentage = 0.25f;
		LowStaminaPercentage = 0.2f;
		LowEnergyPercentage = 0.2f;
		LowShieldPercentage = 0.1f;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaCharacterAttributes : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UArenaCharacterAttributes(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts
	virtual void InitializeComponent() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void Reset(AArenaCharacter* Owner);

	UFUNCTION(BlueprintCallable, Category = Resources)
	void Regenerate(float DeltaSeconds);

	/** get max health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxHealth() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentHealth() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetCurrentHealth(float Value);

	/** get max stamina */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxStamina() const;
	/** get current stamina */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentStamina() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetCurrentStamina(float Value);

	/** get max energy */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxEnergy() const;
	/** get current energy */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentEnergy() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetCurrentEnergy(float Value);

	/** get max shields */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxShields() const;
	/** get current shields */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentShields() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetCurrentShields(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Resources)
	uint32 bIsDying : 1;

	/** get current shields */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetSpeed() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetSpeed(float Value);

	/** get current shields */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetProtection() const;
	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	void SetProtection(float Value);

private:

	AArenaCharacter* Owner;

	UPROPERTY(Transient, Replicated)
	float CurrentShield;
	UPROPERTY(Transient, Replicated)
	float CurrentHealth;
	UPROPERTY(Transient, Replicated)
	float CurrentEnergy;
	UPROPERTY(Transient, Replicated)
	float CurrentStamina;

	/** stats data */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Config)
	FPlayerStats StatsConfig;

	/** resource data */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Config)
	FPlayerResources ResourcesConfig;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(EditDefaultsOnly, Category = Resources)
	uint32 bHasShield : 1;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowHealthSound;
	/** sound played when stamina is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowStaminaSound;
	/** sound played when energy is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowEnergySound;
	/** sound played when shield is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowShieldSound;

	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;
	/** hook to looped low stamina sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowStaminaWarningPlayer;
	/** hook to looped low energy sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowEnergyWarningPlayer;
	/** hook to looped low shield sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowShieldWarningPlayer;

		
	
};
