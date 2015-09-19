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
		Owner->SetHeadUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::UpperBack)
	{
		Owner->SetUpperBackUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::LowerBack)
	{
		Owner->SetLowerBackUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::LeftWrist)
	{
		Owner->SetLeftWristUtility(NewUtility);
	}
	else if (UtilitySelected == EUtilitySelected::RightWrist)
	{
		Owner->SetRightWristUtility(NewUtility);
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

void UArenaCharacterInventory::SwitchArmor()
{
	if (ArmorSelected == EArmorSelected::Head)
	{
		Owner->SetHeadArmor(NewArmor);
	}
	else if (ArmorSelected == EArmorSelected::Chest)
	{
		Owner->SetChestArmor(NewArmor);
	}
	else if (ArmorSelected == EArmorSelected::Shoulder)
	{
		Owner->SetShoulderArmor(NewArmor);
	}
	else if (ArmorSelected == EArmorSelected::Hands)
	{
		Owner->SetHandArmor(NewArmor);
	}
	else if (ArmorSelected == EArmorSelected::Legs)
	{
		Owner->SetLegArmor(NewArmor);
	}
	else if (ArmorSelected == EArmorSelected::Feet)
	{
		Owner->SetFeetArmor(NewArmor);
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

TArray<TSubclassOf<class AArenaArmor>> UArenaCharacterInventory::GetArmorBP()
{
	return DefaultArmorClasses;
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

TSubclassOf<class AArenaArmor> UArenaCharacterInventory::GetNewArmor()
{
	return NewArmor;
}
void UArenaCharacterInventory::SetNewArmor(TSubclassOf<class AArenaArmor> Armor)
{
	NewArmor = Armor;
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

EArmorSelected::Type UArenaCharacterInventory::GetArmorSelected()
{
	return ArmorSelected;
}
void UArenaCharacterInventory::SetArmorSelected(EArmorSelected::Type Selected)
{
	ArmorSelected = Selected;
}