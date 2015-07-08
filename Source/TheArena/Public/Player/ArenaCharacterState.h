// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "ArenaCharacterState.generated.h"

UENUM(BlueprintCallable, BlueprintType, Category = Character)
namespace EPlayerState
{
	enum Type
	{
		Idle,
		Jumping,
		Running,
		Crouching,
		Covering,
		Vaulting,
		Default
	};
}

UENUM(BlueprintCallable, BlueprintType, Category = Character)
namespace ECoverState
{
	enum Type
	{
		HighLeft,
		HighRight,
		HighMiddle,
		LowLeft,
		LowRight,
		LowMiddle,
		Default
	};
}

UENUM(BlueprintCallable, BlueprintType, Category = Character)
namespace ECombatState
{
	enum Type
	{
		Aggressive,
		Passive,
		Default
	};
}

UCLASS()
class THEARENA_API UArenaCharacterState : public UObject
{
	GENERATED_BODY()

	UArenaCharacterState(const class FObjectInitializer& PCIP);

public:

	/** clears everything */
	void Reset();

	/** get current weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	EPlayerState::Type GetPlayerState() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetPlayerState(EPlayerState::Type NewState);

	/** get current weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	ECoverState::Type GetCoverState() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetCoverState(ECoverState::Type NewState);

	/** get current weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	ECombatState::Type GetCombatState() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetCombatState(ECombatState::Type NewState);

protected:

	/** current weapon state */
	EPlayerState::Type PlayerState;

	/** current cover state */
	ECoverState::Type CoverState;

	/** current cover state */
	ECombatState::Type CombatState;
	
};
