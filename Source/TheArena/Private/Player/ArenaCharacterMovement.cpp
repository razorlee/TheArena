// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"

UArenaCharacterMovement::UArenaCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicated(true);
	bReplicates = true;

	BaseTurnRate = 25.f;
	BaseLookUpRate = 25.f;

	MovementSpeedModifier = 1.0f;
	BaseMovementSpeed = 400.0f;
	RunningMovementSpeed = 650.0f;
	CrouchedMovementSpeed = 300.0f;
	TargetingMovementSpeed = 200.0f;

	FaceDirection = FName(TEXT("Right"));
}

float UArenaCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = BaseMovementSpeed  * MovementSpeedModifier;

	AArenaCharacter* ArenaCharacterOwner = Cast<AArenaCharacter>(PawnOwner);
	if (ArenaCharacterOwner)
	{
		if (ArenaCharacterOwner->GetPlayerState()->GetPlayerState() == EPlayerState::Running)
		{
			MaxSpeed = RunningMovementSpeed * MovementSpeedModifier * ArenaCharacterOwner->GetCharacterAttributes()->GetSpeed();
		}
		if (ArenaCharacterOwner->GetPlayerState()->GetPlayerState() == EPlayerState::Crouching)
		{
			MaxSpeed = CrouchedMovementSpeed * MovementSpeedModifier * ArenaCharacterOwner->GetCharacterAttributes()->GetSpeed();
		}
		if (ArenaCharacterOwner->GetCurrentWeapon())
		{
			if (ArenaCharacterOwner->GetCharacterEquipment()->GetDrawCrosshair())
			{
				MaxSpeed = TargetingMovementSpeed * MovementSpeedModifier * ArenaCharacterOwner->GetCharacterAttributes()->GetSpeed();
			}
		}
	}
	UpdateMIDs(MaxSpeed);
	return MaxSpeed;
}

void UArenaCharacterMovement::UpdateMIDs(float Speed) const
{
	AArenaCharacter* ArenaCharacterOwner = Cast<AArenaCharacter>(PawnOwner);
	if (ArenaCharacterOwner)
	{
		for (int32 i = 0; i < ArenaCharacterOwner->GetMeshMIDs().Num(); ++i)
		{
			UMaterialInstanceDynamic* UseMID = ArenaCharacterOwner->GetMeshMIDs()[i];
			if (UseMID)
			{
				if (Velocity.IsZero())
				{
					float MaterialParam = 0.0f;
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Speed: %f"), MaterialParam));
					UseMID->SetScalarParameterValue(TEXT("Speed"), MaterialParam);
				}
				else
				{
					float MaterialParam = Velocity.X > Velocity.Y ? FMath::Abs(Velocity.X / 3000.0f) : FMath::Abs(Velocity.Y / 3000.0f);
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Speed: %f"), MaterialParam));
					UseMID->SetScalarParameterValue(TEXT("Speed"), MaterialParam);
				}
			}
		}
	}
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
			ArenaCharacterOwner->GetCharacterAttributes()->SetCurrentStamina(ArenaCharacterOwner->GetCharacterAttributes()->GetCurrentStamina() - (CostConfig.SprintCost * DeltaSeconds));
		}
		ArenaCharacterOwner->GetCharacterAttributes()->SetCurrentHealth(ArenaCharacterOwner->GetCharacterAttributes()->GetCurrentHealth());
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

float UArenaCharacterMovement::GetMovementSpeedModifier()
{
	return MovementSpeedModifier;
}
void UArenaCharacterMovement::SetMovementSpeedModifier(float Value)
{
	MovementSpeedModifier = Value;
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

UAnimMontage* UArenaCharacterMovement::GetClimbAnimation()
{
	return ClimbAnimation;
}

UAnimMontage* UArenaCharacterMovement::GetHighLeftAnimation(FString Sequence)
{
	if (Sequence == FString(TEXT("Start")))
	{
		return AimHiLeftAnimStart;
	}
	if (Sequence == FString(TEXT("Loop")))
	{
		return AimHiLeftAnimLoop;
	}
	if (Sequence == FString(TEXT("End")))
	{
		return AimHiLeftAnimEnd;
	}
	return NULL;
}

UAnimMontage* UArenaCharacterMovement::GetHighRightAnimation(FString Sequence)
{
	if (Sequence == FString(TEXT("Start")))
	{
		return AimHiRightAnimStart;
	}
	if (Sequence == FString(TEXT("Loop")))
	{
		return AimHiRightAnimLoop;
	}
	if (Sequence == FString(TEXT("End")))
	{
		return AimHiRightAnimEnd;
	}
	return NULL;
}

UAnimMontage* UArenaCharacterMovement::GetLowLeftAnimation(FString Sequence)
{
	if (Sequence == FString(TEXT("Start")))
	{
		return AimLoLeftAnimStart;
	}
	if (Sequence == FString(TEXT("Loop")))
	{
		return AimLoLeftAnimLoop;
	}
	if (Sequence == FString(TEXT("End")))
	{
		return AimLoLeftAnimEnd;
	}
	return NULL;
}

UAnimMontage* UArenaCharacterMovement::GetLowRightAnimation(FString Sequence)
{
	if (Sequence == FString(TEXT("Start")))
	{
		return AimLoRightAnimStart;
	}
	if (Sequence == FString(TEXT("Loop")))
	{
		return AimLoRightAnimLoop;
	}
	if (Sequence == FString(TEXT("End")))
	{
		return AimLoRightAnimEnd;
	}
	return NULL;
}

FName UArenaCharacterMovement::GetDirection()
{
	return FaceDirection;
}
void UArenaCharacterMovement::SetDirection(FName Direction)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FaceDirection = Direction;
	}
	else
	{
		ServerFaceDirection(Direction);
	}
}

FVector UArenaCharacterMovement::GetCoverDirection()
{
	return Direction;
}
void UArenaCharacterMovement::SetCoverDirection(FVector NewDirection)
{
	Direction = NewDirection;
}

FVector UArenaCharacterMovement::GetLocation()
{
	return Location;
}
void UArenaCharacterMovement::SetLocation(FVector NewLocation)
{
	Location = NewLocation;
}

FName UArenaCharacterMovement::GetLowCover()
{
	return LowCover;
}

/////////////////////////////////////////////// Server ///////////////////////////////////////////////

void UArenaCharacterMovement::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UArenaCharacterMovement, FaceDirection);
}

bool UArenaCharacterMovement::ServerFaceDirection_Validate(FName NewFaceDirection)
{
	return true;
}

void UArenaCharacterMovement::ServerFaceDirection_Implementation(FName NewFaceDirection)
{
	FaceDirection = NewFaceDirection;
}