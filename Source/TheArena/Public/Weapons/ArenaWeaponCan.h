// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class THEARENA_API ArenaWeaponCan
{

public:

	/*Checks if the player can turn*/
	static bool Equip(AArenaCharacter* character, AArenaWeapon* weapon);

	/*Checks if the player can look up*/
	static bool UnEquip(AArenaCharacter* character, AArenaWeapon* weapon);

	/*Checks if the player can look up*/
	static bool Activate(AArenaCharacter* character, AArenaWeapon* weapon);

	/** check if weapon can fire */
	static bool Fire(AArenaCharacter* character, AArenaWeapon* weapon);

	/** check if weapon can be reloaded */
	static bool Reload(AArenaCharacter* character, AArenaWeapon* weapon);

	/** check if weapon can be reloaded */
	static bool Melee(AArenaCharacter* character, AArenaWeapon* weapon);

};
