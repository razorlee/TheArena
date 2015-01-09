// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ArenaPlayerController_Menu.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaPlayerController_Menu : public APlayerController
{
	GENERATED_BODY()
	
	/** After game is initialized */
	virtual void PostInitializeComponents() override;
	
};
