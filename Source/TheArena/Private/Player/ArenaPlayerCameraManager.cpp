#include "TheArena.h"

AArenaPlayerCameraManager::AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	TargetingFOV = 60.0f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void AArenaPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	//AArenaCharacter* MyPawn = PCOwner ? Cast<AArenaCharacter>(PCOwner->GetPawn()) : NULL;
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(PCOwner->GetPawn());
	if (MyPawn)
	{
		const float TargetFOV = /*MyPawn->IsTargeting() ? TargetingFOV :*/ NormalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
	}

	Super::UpdateCamera(DeltaTime);

	if (MyPawn)
	{
		//MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());

		EPlayerState::Type PlayerState = MyPawn->GetPlayerState()->GetPlayerState();
		ECoverState::Type CoverState = MyPawn->GetPlayerState()->GetCoverState();
		ECombatState::Type CombatState = MyPawn->GetPlayerState()->GetCombatState();

////////////////////////////////////////// Out of Combat //////////////////////////////////////////

		if (CombatState == ECombatState::Passive)
		{
			MyPawn->bUseControllerRotationYaw = false;
			MyPawn->CameraBoom->TargetArmLength = 250.0f;
			MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
		}

////////////////////////////////////////// In Combat //////////////////////////////////////////

		if (CombatState == ECombatState::Aggressive)
		{
			ETargetingState::Type TargetingState = MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();
			MyPawn->bUseControllerRotationYaw = true;

			if (TargetingState == ETargetingState::Targeting)
			{
				MyPawn->CameraBoom->TargetArmLength = 50.0f;
				MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
			}
			else
			{
				MyPawn->CameraBoom->TargetArmLength = 150.0f;
				MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
			}

			////////////////////////////////////////// Running //////////////////////////////////////////

			if (PlayerState == EPlayerState::Running)
			{
				MyPawn->CameraBoom->TargetArmLength = 175.0f;
				MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 0.0f);
			}

			////////////////////////////////////////// In Cover //////////////////////////////////////////

			if (PlayerState == EPlayerState::Covering)
			{
				ETargetingState::Type WeaponState = MyPawn->GetCharacterEquipment()->GetCurrentWeapon()->GetWeaponState()->GetTargetingState();

				/************************************* In Low Cover *************************************/

				if (CoverState == ECoverState::LowLeft || ECoverState::LowRight || ECoverState::LowMiddle)
				{

					/*************************** In Facing Left ***************************/

					if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
					{
						if (TargetingState == ETargetingState::Targeting)
						{
							if (CoverState == ECoverState::LowLeft)
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, -50.0f, -20.0f);
							}
							else
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
							}
						}
						else
						{
							MyPawn->CameraBoom->TargetArmLength = 150.0f;
							MyPawn->CameraBoom->SocketOffset = FVector(0.0f, -50.0f, -20.0f);
						}
					}

					/*************************** In Facing Right ***************************/

					if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
					{
						if (TargetingState == ETargetingState::Targeting)
						{
							if (CoverState == ECoverState::LowRight)
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, -20.0f);
							}
							else
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
							}
						}
						else
						{
							MyPawn->CameraBoom->TargetArmLength = 150.0f;
							MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, -20.0f);
						}
					}
				}

				/************************************** In High Cover **************************************/

				if (CoverState == ECoverState::HighLeft || ECoverState::HighRight || ECoverState::HighMiddle)
				{

					/*************************** In Facing Left ***************************/

					if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Left")))
					{
						if (TargetingState == ETargetingState::Targeting)
						{
							if (CoverState == ECoverState::HighLeft)
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, -50.0f, 50.0f);
							}
						}
						else
						{
							MyPawn->CameraBoom->TargetArmLength = 150.0f;
							MyPawn->CameraBoom->SocketOffset = FVector(0.0f, -50.0f, 50.0f);
						}
					}

					/*************************** In Facing Right ***************************/

					if (MyPawn->GetPlayerMovement()->GetDirection() == FName(TEXT("Right")))
					{
						if (TargetingState == ETargetingState::Targeting)
						{
							if (CoverState == ECoverState::HighRight)
							{
								MyPawn->CameraBoom->TargetArmLength = 50.0f;
								MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
							}
						}
						else
						{
							MyPawn->CameraBoom->TargetArmLength = 150.0f;
							MyPawn->CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
						}
					}
				}
			}
		}
	}
}
