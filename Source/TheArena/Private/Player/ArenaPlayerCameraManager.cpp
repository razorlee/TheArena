#include "TheArena.h"

AArenaPlayerCameraManager::AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MyPawn = NULL;
	Speed = 20.0f;
	bAlwaysApplyModifiers = true;
}

void AArenaPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);
	MyPawn = Cast<AArenaCharacter>(PCOwner->GetPawn());

	if (MyPawn)
	{
		ECombatState::Type CombatState = MyPawn->GetPlayerState()->GetCombatState();

		if (CombatState == ECombatState::Passive)
		{
			HandlePassiveCamera();
		}
		else
		{
			HandleAggressiveCamera();
		}	
		UpdateCurrents(DeltaTime);
	}
}

void AArenaPlayerCameraManager::UpdateCurrents(float DeltaTime)
{
	if (TargetArm == MyPawn->CameraBoom->TargetArmLength && TargetOffset == MyPawn->CameraBoom->SocketOffset)
	{
		CurrentArm = TargetArm;
		CurrentOffset = TargetOffset;
	}
	else
	{
		MyPawn->CameraBoom->TargetArmLength = FMath::FInterpTo(CurrentArm, TargetArm, DeltaTime, Speed);
		MyPawn->CameraBoom->SocketOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, Speed);
	}
}

////////////////////////////////////////// Camera States //////////////////////////////////////////

void AArenaPlayerCameraManager::HandlePassiveCamera()
{
	MyPawn->bUseControllerRotationYaw = false;
	TargetArm = 250.0f;
	TargetOffset = FVector(0.0f, 0.0f, 0.0f);
}

void AArenaPlayerCameraManager::HandleAggressiveCamera()
{
	EPlayerState::Type PlayerState = MyPawn->GetPlayerState()->GetPlayerState();

	if (MyPawn->GetCharacterEquipment()->GetCurrentWeapon())
	{
		ETargetingState::Type TargetingState = MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();

		Speed = 100 * MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponAttributes()->GetMotility();
		MyPawn->bUseControllerRotationYaw = true;

		if (TargetingState == ETargetingState::Targeting)
		{
			HandleTargetingCamera();
		}
		else if (PlayerState == EPlayerState::Running)
		{
			HandleRunningCamera();
		}
		else if (PlayerState == EPlayerState::Covering)
		{
			HandleCoverCamera();
		}
		else
		{
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, 50.0f, 50.0f);
		}
	}
}

void AArenaPlayerCameraManager::HandleTargetingCamera()
{
	TargetArm = 50.0f;
	TargetOffset = FVector(0.0f, 50.0f, 50.0f);
}

void AArenaPlayerCameraManager::HandleRunningCamera()
{
	TargetArm = 175.0f;
	TargetOffset = FVector(0.0f, 50.0f, 0.0f);
}

void AArenaPlayerCameraManager::HandleCoverCamera()
{
	ECoverState::Type CoverState = MyPawn->GetPlayerState()->GetCoverState();

	if (CoverState == ECoverState::HighLeft || ECoverState::HighRight || ECoverState::HighMiddle)
	{
		HandleHighCoverCamera(CoverState);
	}
	else if (CoverState == ECoverState::LowLeft || ECoverState::LowRight || ECoverState::LowMiddle)
	{
		HandleLowCoverCamera(CoverState);
	}
}

////////////////////////////////////// Cover Camera Handlers //////////////////////////////////////

void AArenaPlayerCameraManager::HandleLowCoverCamera(ECoverState::Type CoverState)
{
	ETargetingState::Type TargetingState = MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();

	if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
	{
		HandleFaceLeftCamera("Low", TargetingState, CoverState);
	}
	else if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
	{
		HandleFaceRightCamera("Low", TargetingState, CoverState);
	}
}

void AArenaPlayerCameraManager::HandleHighCoverCamera(ECoverState::Type CoverState)
{
	ETargetingState::Type TargetingState = MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();

	if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
	{
		HandleFaceLeftCamera("High", TargetingState, CoverState);
	}
	else if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
	{
		HandleFaceRightCamera("High", TargetingState, CoverState);
	}
}

void AArenaPlayerCameraManager::HandleFaceLeftCamera(FString State, ETargetingState::Type TargetingState, ECoverState::Type CoverState)
{
	if (State == "High")
	{
		if (TargetingState == ETargetingState::Targeting)
		{
			if (CoverState == ECoverState::HighLeft)
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, -50.0f, 50.0f);
			}
		}
		else
		{
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, -50.0f, 50.0f);
		}
	}
	else if (State == "Low")
	{
		if (TargetingState == ETargetingState::Targeting)
		{
			if (CoverState == ECoverState::LowLeft)
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, -50.0f, -20.0f);
			}
			else
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, -50.0f, -20.0f);
		}
	}
}

void AArenaPlayerCameraManager::HandleFaceRightCamera(FString State, ETargetingState::Type TargetingState, ECoverState::Type CoverState)
{
	if (State == "High")
	{
		if (TargetingState == ETargetingState::Targeting)
		{
			if (CoverState == ECoverState::HighRight)
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, 50.0f, 50.0f);
		}
	}
	else if (State == "Low")
	{
		if (TargetingState == ETargetingState::Targeting)
		{
			if (CoverState == ECoverState::LowRight)
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, 50.0f, -20.0f);
			}
			else
			{
				TargetArm = 50.0f;
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, 50.0f, -20.0f);
		}
	}
}