// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"

UArenaCharacterMovement::UArenaCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BaseTurnRate = 25.f;
	BaseLookUpRate = 25.f;

	JumpCost = 200.0f;
	SprintCost = 50.0f;
	DodgeCost = 300.0f;

	BaseMovementSpeed = 400.0f;
	RunningMovementSpeed = 520.0f;
	CrouchedMovementSpeed = 340.0f;
	TargetingMovementSpeed = 280.0f;
}

float UArenaCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = BaseMovementSpeed;

	const AArenaCharacter* ArenaCharacterOwner = Cast<AArenaCharacter>(PawnOwner);
	if (ArenaCharacterOwner)
	{
		if (ArenaCharacterOwner->GetPlayerState()->GetPlayerState() == EPlayerState::Running)
		{
			MaxSpeed = RunningMovementSpeed;
		}
		if (ArenaCharacterOwner->GetPlayerState()->GetPlayerState() == EPlayerState::Crouching)
		{
			MaxSpeed = CrouchedMovementSpeed;
		}
	}

	return MaxSpeed;
}

void UArenaCharacterMovement::ManageState(float DeltaSeconds)
{
	AArenaCharacter* ArenaCharacterOwner = Cast<AArenaCharacter>(PawnOwner);
	if (ArenaCharacterOwner)
	{
		if (ArenaCharacterOwner->GetCharacterAttributes()->GetCurrentStamina() <= 0)
		{
			ArenaCharacterOwner->OnStopRunning();
		}

		if (ArenaCharacterOwner->GetPlayerState()->GetPlayerState() == EPlayerState::Running)
		{
			ArenaCharacterOwner->GetCharacterAttributes()->SetCurrentStamina(ArenaCharacterOwner->GetCharacterAttributes()->GetCurrentStamina() - (SprintCost * DeltaSeconds));
		}
	}
}

float UArenaCharacterMovement::GetTurnRate()
{
	return BaseTurnRate;
}

float UArenaCharacterMovement::GetLookUpRate()
{
	return BaseLookUpRate;
}

float UArenaCharacterMovement::GetJumpCost()
{
	return JumpCost;
}

void UArenaCharacterMovement::SetJumpCost(float cost)
{
	JumpCost = cost;
}

USoundBase* UArenaCharacterMovement::GetRunLoopSound()
{
	return RunLoopSound;
}

USoundBase* UArenaCharacterMovement::GetRunStopSound()
{
	return RunStopSound;
}

UAudioComponent* UArenaCharacterMovement::GetRunLoopAC()
{
	return RunLoopAC;
}

void UArenaCharacterMovement::SetRunLoopAC(UAudioComponent* AudioComp)
{
	RunLoopAC = AudioComp;
}

UAnimMontage* UArenaCharacterMovement::GetVaultAnimation()
{
	return VaultAnimation;
}

FName UArenaCharacterMovement::GetDirection()
{
	return FaceDirection;
}

void UArenaCharacterMovement::SetDirection(FName Direction)
{
	FaceDirection = Direction;
}