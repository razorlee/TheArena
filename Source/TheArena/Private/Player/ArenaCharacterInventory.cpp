// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterInventory.h"


// Sets default values for this component's properties
UArenaCharacterInventory::UArenaCharacterInventory()
{
	bWantsInitializeComponent = true;
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

TArray<TSubclassOf<class AArenaWeapon>> UArenaCharacterInventory::GetInventoryBP()
{
	return DefaultInventoryClasses;
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