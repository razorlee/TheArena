// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEARENA_API UArenaCharacterState : public UActorComponent
{
	GENERATED_BODY()

	UArenaCharacterState(const class FObjectInitializer& PCIP);

public:

	/** clears everything */
	void Reset();

	UFUNCTION(BlueprintCallable, Category = Character)
	class AArenaCharacter* GetMyPawn() const;
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetMyPawn(AArenaCharacter* Pawn);

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
	void SetCombatState(ECombatState::Type NewState, class AArenaCharacter* Pawn);

protected:

	UPROPERTY()
	AArenaCharacter* MyPawn;

	/** current weapon state */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
	TEnumAsByte<EPlayerState::Type> PlayerState;

	/** current cover state */
	ECoverState::Type CoverState;

	/** current cover state */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	TEnumAsByte<ECombatState::Type> CombatState;
	
	///////////////////////////////////////////// Replication /////////////////////////////////////////////

	UFUNCTION()
	void OnRep_PlayerState();

	UFUNCTION()
	void OnRep_CombatState(ECombatState::Type NewState);
};
