// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterInventory.h"


// Sets default values for this component's properties
UArenaCharacterInventory::UArenaCharacterInventory()
{
	bWantsInitializeComponent = true;
	//UtilitySelected = EUtilitySelected::Head;
}

void UArenaCharacterInventory::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<AArenaCharacter>(GetOwner());
	SpawnDefaultInventory();

	PrimarySelected = false;
	SecondarySelected = false;
}

void UArenaCharacterInventory::SpawnDefaultInventory()
{
	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.bNoCollisionFail = true;
			AArenaWeapon* NewWeapon = GetWorld()->SpawnActor<AArenaWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			Inventory.AddUnique(NewWeapon);
		}
	}
}

void UArenaCharacterInventory::SwitchWeapon()
{
	if (PrimarySelected)
	{
		Owner->SetPrimaryWeapon(NewWeapon);
	}
	else if (SecondarySelected)
	{
		Owner->SetSecondaryWeapon(NewWeapon);
	}
}

void UArenaCharacterInventory::SwitchUtility()
{

	if (UtilitySelected == EUtilitySelected::Head)
	{
		
	}
	else if (UtilitySelected == EUtilitySelected::UpperBack)
	{
		Owner->SetUpperBackUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::LowerBack)
	{

	}
	else if (UtilitySelected == EUtilitySelected::LeftWrist)
	{

	}
	else if (UtilitySelected == EUtilitySelected::RightWrist)
	{

	}
	else if (UtilitySelected == EUtilitySelected::LeftWaist)
	{
		Owner->SetLeftWaistUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::RightWaist)
	{
		Owner->SetRightWaistUtility(NewUtility);
	}
}

TArray<TSubclassOf<class AArenaWeapon>> UArenaCharacterInventory::GetInventoryBP()
{
	return DefaultInventoryClasses;
}

TArray<TSubclassOf<class AArenaUtility>> UArenaCharacterInventory::GetUtilitiesBP()
{
	return DefaultUtilityClasses;
}

TArray<class AArenaWeapon*> UArenaCharacterInventory::GetInventory()
{
	return Inventory;
}

TSubclassOf<class AArenaWeapon> UArenaCharacterInventory::GetNewWeapon()
{
	return NewWeapon;
}
void UArenaCharacterInventory::SetNewWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	NewWeapon = Weapon;
}

TSubclassOf<class AArenaUtility> UArenaCharacterInventory::GetNewUtility()
{
	return NewUtility;
}
void UArenaCharacterInventory::SetNewUtility(TSubclassOf<class AArenaUtility> Utility)
{
	NewUtility = Utility;
}

bool UArenaCharacterInventory::GetPrimarySelected()
{
	return PrimarySelected;
}
void UArenaCharacterInventory::SetPrimarySelected(bool Selected)
{
	PrimarySelected = Selected;
}

bool UArenaCharacterInventory::GetSecondarySelected()
{
	return SecondarySelected;
}
void UArenaCharacterInventory::SetSecondarySelected(bool Selected)
{
	SecondarySelected = Selected;
}

EUtilitySelected::Type UArenaCharacterInventory::GetUtilitySelected()
{
	return UtilitySelected;
}
void UArenaCharacterInventory::SetUtilitySelected(EUtilitySelected::Type Selected)
{
	UtilitySelected = Selected;
}