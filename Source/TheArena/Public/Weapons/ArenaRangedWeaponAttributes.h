// Fill out your copyright notice in the Description page of Project void Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaRangedWeaponAttributes.generated.h"

USTRUCT()
struct FWeaponStats
{
	GENERATED_USTRUCT_BODY()

	/** The damage dealt to player before defense mitigation */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "1", ClampMax = "1000", UIMin = "1", UIMax = "1000"))
	int32 Damage;

	/** The precent of the by which recoil is reduced*/
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	float Stability;

	/** The rate at which weapon handling is performed */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "0.5", ClampMax = "2", UIMin = "0.5", UIMax = "2"))
	float Motility;

	/** The velocity of the projectile in Unreal Units */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "5000", ClampMax = "45000", UIMin = "5000", UIMax = "45000"))
	float Velocity;

	/** The spread of weapon fire */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "0", ClampMax = "25", UIMin = "0", UIMax = "25"))
	float Accuracy;

	/** The time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "0.01", ClampMax = "2", UIMin = "0.01", UIMax = "2"))
	float AttackSpeed;

	/** Increases the speed of dodging*/
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float Mobility;

	/** The number of rounds in a clip */
	UPROPERTY(EditDefaultsOnly, Category = Stats, meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
	int32 Capacity;

	/** defaults */
	FWeaponStats()
	{
		Damage = 100;
		Stability = 5.0f;
		Motility = 100.0f;
		Velocity = 37600.0f;
		Accuracy = 0.25f;
		AttackSpeed = 0.25f;
		Mobility = 0.0f;
		Capacity = 20.0f;
	}
};

UENUM()
namespace EFireMode
{
	enum Type 
	{
		Automatic	UMETA(DisplayName = "Automatic"),
		SemiAuto	UMETA(DisplayName = "Semi-Automatic"),
		Burst		UMETA(DisplayName = "Burst Fire")
	};
}

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaRangedWeaponAttributes : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(BlueprintReadOnly)
	float TotalAmmo;

	UPROPERTY(BlueprintReadOnly)
	float CurrentClip;

	UPROPERTY(BlueprintReadOnly)
	float BurstCounter;

	UPROPERTY(BlueprintReadOnly)
	float LastFireTime;

	// void Sets default values for this component's properties
	UArenaRangedWeaponAttributes();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 GetDamage();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetDamage(int32 Value);
		
	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetStability();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetStability(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetMotility();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetMotility(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetVelocity();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetVelocity(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetAccuracy();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetAccuracy(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetAttackSpeed();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetAttackSpeed(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetMobility();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetMobility(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 GetCapacity();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetCapacity(int32 Value);

	UFUNCTION(BlueprintCallable, Category = Config)
	EFireMode::Type GetFireMode();
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetFireMode(EFireMode::Type NewMode);

	UFUNCTION(BlueprintCallable, Category = Config)
	int32 GetShotgunPellets();
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetShotgunPellets(int32 Value);

	UFUNCTION(BlueprintCallable, Category = Config)
	bool GetIsExplosive();
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetIsExplosive(bool Value);

	UFUNCTION(BlueprintCallable, Category = Config)
	int32 GetExplosionRadius();
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetExplosionRadius(int32 Value);

	UFUNCTION(BlueprintCallable, Category = Config)
	TSubclassOf<UDamageType> GetDamageType();

private:

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponStats WeaponAttributes;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EFireMode::Type> FireMode;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Type")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	int32 ShotgunPellets;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool IsExplosive;

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (EditCondition = "IsExplosive"))
	int32 ExplosionRadius;
};
