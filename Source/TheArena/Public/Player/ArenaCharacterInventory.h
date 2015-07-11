// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterInventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaCharacterInventory : public UActorComponent
{
	GENERATED_BODY()

public:	

	UArenaCharacterInventory();

	virtual void InitializeComponent() override;

	void SpawnDefaultInventory();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TArray<TSubclassOf<class AArenaWeapon>> GetInventoryBP();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TArray<class AArenaWeapon*> GetInventory();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TSubclassOf<class AArenaWeapon> GetNewWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetNewWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetPrimarySelected();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetPrimarySelected(bool Selected);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetSecondarySelected();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetSecondarySelected(bool Selected);


protected:

	UPROPERTY()
	AArenaCharacter* Owner;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaWeapon>> DefaultInventoryClasses;

	UPROPERTY()
	TArray<class AArenaWeapon*> Inventory;

	UPROPERTY()
	TSubclassOf<class AArenaWeapon> NewWeapon;

	UPROPERTY()
	bool PrimarySelected;

	UPROPERTY()
	bool SecondarySelected;
};
