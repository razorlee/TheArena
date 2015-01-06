// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"

float UArenaCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AArenaCharacter* ArenaCharacterOwner = Cast<AArenaCharacter>(PawnOwner);
	if (ArenaCharacterOwner)
	{
		if (ArenaCharacterOwner->IsTargeting())
		{
			MaxSpeed = 300.0f;
		}
		if (ArenaCharacterOwner->IsRunning())
		{
			MaxSpeed = 600.0f;
		}
	}

	return MaxSpeed;
}



