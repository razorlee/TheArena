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

UENUM(BlueprintType)
namespace EArmorSelected
{
	enum Type
	{
		Head		UMETA(DisplayName = "Head"),
		Shoulder	UMETA(DisplayName = "Shoulder"),
		Chest		UMETA(DisplayName = "Chest"),
		Hands		UMETA(DisplayName = "Hands"),
		Legs		UMETA(DisplayName = "Legs"),
		Feet		UMETA(DisplayName = "Feet"),
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

/////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Category = Utility)
	void SwitchUtility();

	UFUNCTION(BlueprintCallable, Category = Armor)
	void SwitchArmor();

/////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TArray<TSubclassOf<class AArenaWeapon>> GetInventoryBP();

	UFUNCTION(BlueprintCallable, Category = Utility)
	TArray<TSubclassOf<class AArenaUtility>> GetUtilitiesBP();

	UFUNCTION(BlueprintCallable, Category = Armor)
	TArray<TSubclassOf<class AArenaArmor>> GetArmorBP();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TArray<class AArenaWeapon*> GetInventory();

////////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Weapon)
	TSubclassOf<class AArenaWeapon> GetNewWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetNewWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Utility)
	TSubclassOf<class AArenaUtility> GetNewUtility();
	UFUNCTION(BlueprintCallable, Category = Utility)
	void SetNewUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Armor)
	TSubclassOf<class AArenaArmor> GetNewArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
	void SetNewArmor(TSubclassOf<class AArenaArmor> Armor);

//////////////////////////////////////////////////////////////////////////

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

	UFUNCTION(BlueprintCallable, Category = Utility)
	EArmorSelected::Type GetArmorSelected();
	UFUNCTION(BlueprintCallable, Category = Utility)
	void SetArmorSelected(EArmorSelected::Type Selected);

protected:

	UPROPERTY()
	AArenaCharacter* Owner;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaWeapon>> DefaultInventoryClasses;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaUtility>> DefaultUtilityClasses;

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaArmor>> DefaultArmorClasses;

	UPROPERTY()
	TArray<class AArenaWeapon*> Inventory;

	UPROPERTY()
	TSubclassOf<class AArenaWeapon> NewWeapon;

	UPROPERTY()
	TSubclassOf<class AArenaUtility> NewUtility;

	UPROPERTY()
	TSubclassOf<class AArenaArmor> NewArmor;

	UPROPERTY()
	bool PrimarySelected;

	UPROPERTY()
	bool SecondarySelected;

	UPROPERTY()
	TEnumAsByte<EUtilitySelected::Type> UtilitySelected;

	UPROPERTY()
	TEnumAsByte<EArmorSelected::Type> ArmorSelected;
};
