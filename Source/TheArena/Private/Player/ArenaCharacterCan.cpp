// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterCan.h"

bool ArenaCharacterCan::Turn(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Vaulting)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::LookUp(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::MoveForward(AArenaCharacter* character, AArenaPlayerController* controller, float Value)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Covering
		&& Value != 0.0f)
	{
		if (Value < 0.0f)
		{
			if (character->GetPlayerState()->GetPlayerState() != EPlayerState::Running)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::MoveRight(AArenaCharacter* character, AArenaPlayerController* controller, float Value)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Running
		&& Value != 0.0f)
	{
		if (Value < 0.0f)
		{
			if (character->GetPlayerState()->GetPlayerState() != EPlayerState::Covering 
				|| (character->GetPlayerState()->GetPlayerState() == EPlayerState::Covering && !character->GetPlayerState()->GetIsNearLeftEdge()))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (character->GetPlayerState()->GetPlayerState() != EPlayerState::Covering 
				|| (character->GetPlayerState()->GetPlayerState() == EPlayerState::Covering && !character->GetPlayerState()->GetIsNearRightEdge()))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Run(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFlying()
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetVelocity().IsZero()
		&& character->GetCharacterAttributes()->GetCurrentStamina() > 200
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool ArenaCharacterCan::Crouch(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetCharacterMovement()->IsFlying()
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Jumping
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Jump(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetCharacterAttributes()->GetCurrentStamina() >= character->GetPlayerMovement()->CostConfig.JumpCost
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetCharacterMovement()->IsFlying()
		&& !character->GetPlayerState()->GetIsNearCover())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Cover(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetCharacterMovement()->IsFlying()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& character->GetPlayerState()->GetIsNearCover())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Vault(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetCharacterMovement()->IsFlying()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& character->GetPlayerState()->GetIsNearCover()
		//&& !character->GetPlayerState()->GetIsNearLeftEdge()
		//&& !character->GetPlayerState()->GetIsNearRightEdge()
		&& character->GetPlayerState()->GetCanVault())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Climb(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFalling()
		&& !character->GetCharacterMovement()->IsFlying()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& character->GetPlayerState()->GetIsNearCover()
		//&& !character->GetPlayerState()->GetIsNearLeftEdge()
		//&& !character->GetPlayerState()->GetIsNearRightEdge()
		&& character->GetPlayerState()->GetCanClimb())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Equip(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Swap(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetCombatState() == ECombatState::Aggressive
		&& character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() != EWeaponState::Equipping
		&& character->GetCurrentWeapon() != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Target(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Running)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Peak(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetCurrentWeapon()
		&& character->GetPlayerState()->GetCombatState() == ECombatState::Aggressive
		&& character->GetPlayerState()->GetPlayerState() == EPlayerState::Covering)
	{
		if (!character->GetCurrentWeapon()->GetWeaponState()->GetCoverTargeting())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::ToggleCombat(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPrimaryWeapon()
		&& character->GetSecondaryWeapon()
		&& (character->GetPlayerState()->GetPlayerState() == EPlayerState::Default
		|| character->GetPlayerState()->GetPlayerState() == EPlayerState::Jumping))
	{
		if (character->GetCurrentWeapon())
		{
			if (character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() == EWeaponState::Default
				&& character->GetCurrentWeapon()->GetWeaponState()->GetTargetingState() == ETargetingState::Default)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Fire(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetCurrentWeapon()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Running
		&& character->GetPlayerState()->GetPlayerState() != EPlayerState::Vaulting)
	{
		if (character->GetPlayerState()->GetPlayerState() == EPlayerState::Covering)
		{
			if (character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() != EWeaponState::Reloading && character->Peaking)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() != EWeaponState::Reloading)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Head(AArenaCharacter* character, AArenaPlayerController* controller)
{
	return true;
}

bool ArenaCharacterCan::Back(AArenaCharacter* character, AArenaPlayerController* controller)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Controller: %s"), controller ? TEXT("True") : TEXT("False")));
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetUpperBackUtility()
		&& character->GetPlayerState()->GetCombatState() == ECombatState::Aggressive)
	{
		if (!character->GetUpperBackUtility()->Active)
		{
			if (character->GetCharacterAttributes()->GetCurrentEnergy() >= character->GetUpperBackUtility()->GetActivationCost())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Wrist(AArenaCharacter* character, AArenaPlayerController* controller)
{
	return true;
}

bool ArenaCharacterCan::Waist(AArenaCharacter* character, AArenaPlayerController* controller)
{
	return true;
}

bool ArenaCharacterCan::Reload(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive
		&& (character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() == EWeaponState::Idle
		|| character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() == EWeaponState::Default
		|| character->GetCurrentWeapon()->GetWeaponState()->GetWeaponState() == EWeaponState::Firing))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Melee(AArenaCharacter* character, AArenaPlayerController* controller)
{
	if (controller
		&& controller->IsGameInputAllowed()
		&& !character->GetCharacterMovement()->IsFlying()
		&& !character->GetCharacterMovement()->IsFalling()
		&& character->GetPlayerState()->GetCombatState() != ECombatState::Passive)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArenaCharacterCan::Die(AArenaCharacter* character)
{
	if (character->GetCharacterAttributes()->bIsDying
		|| character->IsPendingKill()
		|| character->Role != ROLE_Authority
		|| character->GetWorld()->GetAuthGameMode() == NULL
		|| character->GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)
	{
		return false;
	}

	return true;
}