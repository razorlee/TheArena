// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterEquipment.h"


// Sets default values for this component's properties
UArenaCharacterEquipment::UArenaCharacterEquipment(const FObjectInitializer& ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;


}

void UArenaCharacterEquipment::InitializeComponent()
{
	Super::InitializeComponent();

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(PrimaryWeaponBP, SpawnInfo);
	PrimaryWeapon->SetOwningPawn(Cast<AArenaCharacter>(GetOwner()));
	PrimaryWeapon->SetPrimary(true);
	PrimaryWeapon->UnEquip();

	SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(SecondaryWeaponBP, SpawnInfo);
	SecondaryWeapon->SetOwningPawn(Cast<AArenaCharacter>(GetOwner()));
	SecondaryWeapon->SetPrimary(false);
	SecondaryWeapon->UnEquip();
}

void UArenaCharacterEquipment::Reset()
{

}

AArenaWeapon* UArenaCharacterEquipment::GetCurrentWeapon()
{
	return CurrentWeapon;
}
void UArenaCharacterEquipment::SetCurrentWeapon()
{
	if (CurrentWeapon == NULL)
	{
		CurrentWeapon = SecondaryWeapon;
		return;
	}
	if (CurrentWeapon == PrimaryWeapon)
	{
		CurrentWeapon = SecondaryWeapon;
		return;
	}
	if (CurrentWeapon == SecondaryWeapon)
	{
		CurrentWeapon = PrimaryWeapon;
		return;
	}
}

AArenaWeapon* UArenaCharacterEquipment::GetPrimaryWeapon()
{
	return PrimaryWeapon;
}
void UArenaCharacterEquipment::SetPrimaryWeapon(AArenaWeapon* Weapon)
{
	PrimaryWeapon = Weapon;
}

AArenaWeapon* UArenaCharacterEquipment::GetSecondaryWeapon()
{
	return SecondaryWeapon;
}
void UArenaCharacterEquipment::SetSecondaryWeapon(AArenaWeapon* Weapon)
{
	SecondaryWeapon = Weapon;
}

FName UArenaCharacterEquipment::GetWeaponAttachPoint()
{
	return WeaponAttachPoint;
}

FName UArenaCharacterEquipment::GetOffHandAttachPoint()
{
	return OffHandAttachPoint;
}

FName UArenaCharacterEquipment::GetMainWeaponAttachPoint()
{
	return MainWeaponAttachPoint;
}

FName UArenaCharacterEquipment::GetOffWeaponAttachPoint()
{
	return OffWeaponAttachPoint;
}

FName UArenaCharacterEquipment::GetWristOneAttachPoint()
{
	return WristOneAttachPoint;
}

// Called every frame
void UArenaCharacterEquipment::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

