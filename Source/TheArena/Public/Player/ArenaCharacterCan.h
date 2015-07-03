// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaCharacter.h"
#include "ArenaPlayerController.h"

class THEARENA_API ArenaCharacterCan
{

public:

	/*Checks if the player can turn*/
	static bool Turn(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can look up*/
	static bool LookUp(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can look up*/
	static bool MoveForward(AArenaCharacter* character, AArenaPlayerController* controller, float Value);

	/*Checks if the player can look up*/
	static bool MoveRight(AArenaCharacter* character, AArenaPlayerController* controller, float Value);

	/*Checks if the player can sprint*/
	static bool Run(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can crouch*/
	static bool Crouch(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can jump*/
	static bool Jump(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can enter cover*/
	static bool Cover(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can vault over obsticals*/
	static bool Vault(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can equip a weapon*/
	static bool Equip(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can swap weapons*/
	static bool Swap(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can aim a weapon*/
	static bool Target(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can fire a weapon*/
	static bool Fire(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can fire a weapon*/
	static bool Reload(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can fire a weapon*/
	static bool Melee(AArenaCharacter* character, AArenaPlayerController* controller);

	/*Checks if the player can fire a weapon*/
	static bool Die(AArenaCharacter* character);

};
