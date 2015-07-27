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
		Climbing,
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
	bool GetIsNearCover() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetIsNearCover(bool Value);

	/** get current weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	bool GetIsNearLeftEdge() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetIsNearLeftEdge(bool Value);

	/** get current weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	bool GetIsNearRightEdge() const;
	/** update weapon state */
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetIsNearRightEdge(bool Value);

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

	UPROPERTY()
	AArenaCharacter* MyPawn;

	UPROPERTY()
	bool IsNearCover;

	UPROPERTY()
	bool IsNearLeftEdge;

	UPROPERTY()
	bool IsNearRightEdge;

	UPROPERTY(Transient, Replicated)
	TEnumAsByte<EPlayerState::Type> PlayerState;

	UPROPERTY(Transient, Replicated)
	TEnumAsByte<ECoverState::Type> CoverState;

	UPROPERTY(Transient, Replicated)
	TEnumAsByte<ECombatState::Type> CombatState;
};
