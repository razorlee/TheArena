// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Props/ArenaInteractiveObject.h"
#include "ArenaResupplyCrate.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaResupplyCrate : public AArenaInteractiveObject
{
	GENERATED_BODY()
	
public:

	// Constructor to set defaults
	AArenaResupplyCrate();

	// Override on interact event
	void OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate = 1.f) override;
	
	////////////// Getters & Setters /////////////////

	UFUNCTION(BlueprintPure, Category = "Resupply")
	int32 GetAmmoResupply();

	UFUNCTION(BlueprintCallable, Category = "Resupply")
	void SetAmmoResupply(int32 resupplyAmount);

	UFUNCTION(BlueprintPure, Category = "Resupply")
	float GetResupplyDelay();

	UFUNCTION(BlueprintCallable, Category = "Resupply")
	void SetResupplyDelay(float resupplyDelay);

protected:

	// Amount of ammo to give player
	UPROPERTY(EditDefaultsOnly, Category = "Resupply", meta = (BlueprintProtected = "true"))
	int32 AmmoResupply;

	// Length of time before a player can resupply again
	UPROPERTY(EditDefaultsOnly, Category = "Resupply", meta = (BlueprintProtected = "true"))
	float DelayLength;

private:

	
};
