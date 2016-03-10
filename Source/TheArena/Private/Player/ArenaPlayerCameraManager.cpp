#include "TheArena.h"

AArenaPlayerCameraManager::AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MyPawn = NULL;
	Speed = 5.0f;
	NormalFOV = 90.0f;

	CurrentFOV = 90.0;
	CurrentArm = 500.0f;
	CurrentOffset = FVector(0.0f, 0.0f, 0.0f);

	TargetFOV = 90.0f;
	TargetArm = 250.0f;
	TargetOffset = FVector(0.0f, 0.0f, 0.0f);

	CurShoulder = RIGHT_SHOULDER;

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
	if (TargetArm != CurrentArm || TargetOffset != CurrentOffset || TargetFOV != CurrentFOV)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, Speed);
		CurrentArm = FMath::FInterpTo(CurrentArm, TargetArm, DeltaTime, Speed);
		CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, Speed);

		MyPawn->FollowCamera->FieldOfView = CurrentFOV;
		MyPawn->CameraBoom->TargetArmLength = CurrentArm;
		MyPawn->CameraBoom->SocketOffset = CurrentOffset;
	}
}

////////////////////////////////////////// Camera States //////////////////////////////////////////

void AArenaPlayerCameraManager::ToggleShoulder()
{
	if (CurShoulder == RIGHT_SHOULDER)
	{
		CurShoulder = LEFT_SHOULDER;
	}
	else
	{
		CurShoulder = RIGHT_SHOULDER;
	}
}

void AArenaPlayerCameraManager::HandlePassiveCamera()
{
	MyPawn->bUseControllerRotationYaw = false;
	Speed = 5.0f;
	TargetFOV = NormalFOV;
	TargetArm = 250.0f;
	TargetOffset = FVector(0.0f, 0.0f, 0.0f);
}

void AArenaPlayerCameraManager::HandleAggressiveCamera()
{
	EPlayerState::Type PlayerState = MyPawn->GetPlayerState()->GetPlayerState();

	if (MyPawn->GetCurrentWeapon())
	{
		//ETargetingState::Type TargetingState = MyPawn->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();

		Speed = 5.0f * MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetMotility();
		MyPawn->bUseControllerRotationYaw = true;

		if (PlayerState == EPlayerState::Covering)
		{
			HandleCoverCamera();
		}
		else if (PlayerState == EPlayerState::Climbing || PlayerState == EPlayerState::Vaulting)
		{
			MyPawn->bUseControllerRotationYaw = false;
			TargetFOV = NormalFOV;
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, CurShoulder * 50.0f, 50.0f);
		}
		else if (MyPawn->GetTargeting())
		{
			HandleTargetingCamera();
		}
		else if (PlayerState == EPlayerState::Running)
		{
			HandleRunningCamera();
		}
		else
		{
			TargetFOV = NormalFOV;
			TargetArm = 150.0f;
			TargetOffset = FVector(0.0f, CurShoulder * 50.0f, 50.0f);
		}
	}
}

void AArenaPlayerCameraManager::HandleTargetingCamera()
{
	TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
	if (MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetHasScope())
	{
		TargetArm = -30.0f;
		TargetOffset = FVector(0.0f, CurShoulder * 15.0f, 50.0f);
	}
	else
	{
		TargetArm = 50.0f;
		TargetOffset = FVector(0.0f, CurShoulder * 50.0f, 50.0f);
	}
}

void AArenaPlayerCameraManager::HandleRunningCamera()
{
	TargetFOV = NormalFOV;
	TargetArm = 175.0f;
	TargetOffset = FVector(0.0f, CurShoulder * 50.0f, 0.0f);
}

void AArenaPlayerCameraManager::HandleCoverCamera()
{
	ECoverState::Type CoverState = MyPawn->GetPlayerState()->GetCoverState();

	if ((CoverState == ECoverState::HighLeft) || (CoverState == ECoverState::HighRight) || (CoverState == ECoverState::HighMiddle))
	{
		HandleHighCoverCamera(CoverState);
	}
	else if ((CoverState == ECoverState::LowLeft) || (CoverState == ECoverState::LowRight) || (CoverState == ECoverState::LowMiddle))
	{
		HandleLowCoverCamera(CoverState);
	}
}

////////////////////////////////////// Cover Camera Handlers //////////////////////////////////////

void AArenaPlayerCameraManager::HandleLowCoverCamera(ECoverState::Type CoverState)
{
	
	if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
	{
		HandleFaceLeftCamera("Low", CoverState);
	}
	else if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
	{
		HandleFaceRightCamera("Low", CoverState);
	}
}

void AArenaPlayerCameraManager::HandleHighCoverCamera(ECoverState::Type CoverState)
{
	if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
	{
		HandleFaceLeftCamera("High", CoverState);
	}
	else if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
	{
		HandleFaceRightCamera("High", CoverState);
	}
}

void AArenaPlayerCameraManager::HandleFaceLeftCamera(FString State, ECoverState::Type CoverState)
{
	if (State == "High")
	{
		if (MyPawn->GetTargeting())
		{
			MyPawn->bUseControllerRotationYaw = true;
			if (CoverState == ECoverState::HighLeft)
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, -50.0f, 50.0f);
			}
		}
		else
		{
			MyPawn->bUseControllerRotationYaw = false;
			TargetArm = 150.0f;
			TargetFOV = NormalFOV;
			TargetOffset = FVector(0.0f, -50.0f, 50.0f);
		}
	}
	else if (State == "Low")
	{
		if (MyPawn->GetTargeting())
		{
			MyPawn->bUseControllerRotationYaw = true;
			if (CoverState == ECoverState::LowLeft)
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, -50.0f, -20.0f);
			}
			else
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			MyPawn->bUseControllerRotationYaw = false;
			TargetArm = 150.0f;
			TargetFOV = NormalFOV;
			TargetOffset = FVector(0.0f, -50.0f, -20.0f);
		}
	}
}

void AArenaPlayerCameraManager::HandleFaceRightCamera(FString State, ECoverState::Type CoverState)
{
	if (State == "High")
	{
		if (MyPawn->GetTargeting())
		{
			MyPawn->bUseControllerRotationYaw = true;
			if (CoverState == ECoverState::HighRight)
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			MyPawn->bUseControllerRotationYaw = false;
			TargetArm = 150.0f;
			TargetFOV = NormalFOV;
			TargetOffset = FVector(0.0f, 50.0f, 50.0f);
		}
	}
	else if (State == "Low")
	{
		if (MyPawn->GetTargeting())
		{
			MyPawn->bUseControllerRotationYaw = true;
			if (CoverState == ECoverState::LowRight)
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, 50.0f, -20.0f);
			}
			else
			{
				TargetArm = 50.0f;
				TargetFOV = MyPawn->GetCurrentWeapon()->GetWeaponAttributes()->GetZoomFOV();
				TargetOffset = FVector(0.0f, 50.0f, 50.0f);
			}
		}
		else
		{
			MyPawn->bUseControllerRotationYaw = false;
			TargetArm = 150.0f;
			TargetFOV = NormalFOV;
			TargetOffset = FVector(0.0f, 50.0f, -20.0f);
		}
	}
}

////////////////////////////////////// Getters and Setters //////////////////////////////////////

float AArenaPlayerCameraManager::GetNormalFOV()
{
	return NormalFOV;
}
void AArenaPlayerCameraManager::SetNormalFOV(float Value)
{
	NormalFOV = Value;
}
