// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterInventory.generated.h"

UENUM(BlueprintType)
namespace EUtilitySelected
{
	enum Type
	{
		Head		UMETA(DisplayName = "Head"),
		UpperBack	UMETA(DisplayName = "Upper Back"),
		LowerBack	UMETA(DisplayName = "Lower Back"),
		LeftWrist	UMETA(DisplayName = "Left Wrist"),
		RightWrist	UMETA(DisplayName = "Right Wrist"),
		LeftWaist	UMETA(DisplayName = "Left Waist"),
		RightWaist	UMETA(DisplayName = "Right Waist")
	};
}

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

	UFUNCTION(BlueprintCallable, Category = Utility)
	TArray<TSubclassOf<class AArenaUtility>> GetUtilitiesBP();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TArray<class AArenaWeapon*> GetInventory();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TSubclassOf<class AArenaWeapon> GetNewWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetNewWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Utility)
	TSubclassOf<class AArenaUtility> GetNewUtility();
	UFUNCTION(BlueprintCallable, Category = Utility)
	void SetNewUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetPrimarySelected();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetPrimarySelected(bool Selected);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetSecondarySelected();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetSecondarySelected(bool Selected);

	UFUNCTION(BlueprintCallable, Category = Utility)
	EUtilitySelected::Type GetUtilitySelected();
	UFUNCTION(BlueprintCallable, Category = Utility)
	void SetUtilitySelected(EUtilitySelected::Type Selected);

protected:

	UPROPERTY()
	AArenaCharacter* Owner;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaWeapon>> DefaultInventoryClasses;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaUtility>> DefaultUtilityClasses;

	UPROPERTY()
	TArray<class AArenaWeapon*> Inventory;

	UPROPERTY()
	TSubclassOf<class AArenaWeapon> NewWeapon;

	UPROPERTY()
	TSubclassOf<class AArenaUtility> NewUtility;

	UPROPERTY()
	bool PrimarySelected;

	UPROPERTY()
	bool SecondarySelected;

	UPROPERTY()
	TEnumAsByte<EUtilitySelected::Type> UtilitySelected;
};
