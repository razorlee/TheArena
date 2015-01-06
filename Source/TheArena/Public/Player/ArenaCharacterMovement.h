// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API UArenaCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual float GetMaxSpeed() const override;
	
};
