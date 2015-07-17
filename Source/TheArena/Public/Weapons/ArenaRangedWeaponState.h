// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaRangedWeaponState.generated.h"

UENUM(BlueprintCallable, BlueprintType, Category = Weapon)
namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
		Holstering,
		Meleeing,
		Default
	};
}

UENUM(BlueprintCallable, BlueprintType, Category = Weapon)
namespace ETargetingState
{
	enum Type
	{
		Targeting,
		Scoping,
		Default
	};
}

UENUM(BlueprintCallable, BlueprintType, Category = Weapon)
namespace EEquippedState
{
	enum Type
	{
		Equipped,
		UnEquipped,
		Default
	};
}

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEARENA_API UArenaRangedWeaponState : public UActorComponent
{
	GENERATED_BODY()

	UArenaRangedWeaponState(const class FObjectInitializer& PCIP);

public:

	UFUNCTION(BlueprintCallable, Category = Weapon)
	EWeaponState::Type GetWeaponState();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetWeaponState(EWeaponState::Type NewState);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	ETargetingState::Type GetTargetingState();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetTargetingState(ETargetingState::Type NewState);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	EEquippedState::Type GetEquippedState();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetEquippedState(EEquippedState::Type NewState);
	
private:

	/** current weapon state */
	UPROPERTY(Transient, Replicated)
	TEnumAsByte<EWeaponState::Type> WeaponState;

	/** current cover state */
	UPROPERTY(Transient, Replicated)
	TEnumAsByte<ETargetingState::Type> TargetingState;

	/** current cover state */
	UPROPERTY(Transient, Replicated)
	TEnumAsByte<EEquippedState::Type> EquippedState;
	
};
