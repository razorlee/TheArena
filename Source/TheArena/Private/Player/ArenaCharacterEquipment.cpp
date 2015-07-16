// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "Net/UnrealNetwork.h"
#include "ArenaCharacterEquipment.h"


// Sets default values for this component's properties
UArenaCharacterEquipment::UArenaCharacterEquipment(const FObjectInitializer& ObjectInitializer)
{
	SetIsReplicated(true);
	bReplicates = true;

	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UArenaCharacterEquipment::InitializeComponent()
{
	Super::InitializeComponent();

	//FActorSpawnParameters SpawnInfo;
	//SpawnInfo.bNoCollisionFail = true;

	//PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(PrimaryWeaponBP, SpawnInfo);
	//PrimaryWeapon->SetOwningPawn(Cast<AArenaCharacter>(GetOwner()));
	//PrimaryWeapon->SetPrimary(true);
	//PrimaryWeapon->UnEquip();

	//SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(SecondaryWeaponBP, SpawnInfo);
	//SecondaryWeapon->SetOwningPawn(Cast<AArenaCharacter>(GetOwner()));
	//SecondaryWeapon->SetPrimary(false);
	//SecondaryWeapon->UnEquip();

}

void UArenaCharacterEquipment::Reset()
{

}

class AArenaCharacter* UArenaCharacterEquipment::GetMyPawn() const
{
	return MyPawn;
}
void UArenaCharacterEquipment::SetMyPawn(AArenaCharacter* Pawn)
{
	MyPawn = Pawn;
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
void UArenaCharacterEquipment::SetPrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	CurrentWeapon = NULL;
	PrimaryWeapon->Destroy();
	//GetWorld()->DestroyActor(PrimaryWeapon);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(Weapon, SpawnInfo);
	PrimaryWeapon->SetOwningPawn(MyPawn);
	PrimaryWeapon->SetPrimary(true);
	PrimaryWeapon->UnEquip();

	PrimaryWeaponBP = Weapon;
}

AArenaWeapon* UArenaCharacterEquipment::GetSecondaryWeapon()
{
	return SecondaryWeapon;
}
void UArenaCharacterEquipment::SetSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	CurrentWeapon = NULL;
	SecondaryWeapon->Destroy();
	//GetWorld()->DestroyActor(SecondaryWeapon);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(Weapon, SpawnInfo);
	SecondaryWeapon->SetOwningPawn(MyPawn);
	SecondaryWeapon->SetPrimary(false);
	SecondaryWeapon->UnEquip();

	SecondaryWeaponBP = Weapon;
}

bool UArenaCharacterEquipment::GetDrawCrosshair()
{
	return DrawCrosshair;
}
void UArenaCharacterEquipment::SetDrawCrosshair(bool Allow)
{
	DrawCrosshair = Allow;
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

FName UArenaCharacterEquipment::GetMainHeavyAttachPoint()
{
	return MainHeavyAttachPoint;
}
FName UArenaCharacterEquipment::GetOffHeavyAttachPoint()
{
	return OffHeavyAttachPoint;
}

FName UArenaCharacterEquipment::GetMainPistolAttachPoint()
{
	return MainPistolAttachPoint;
}
FName UArenaCharacterEquipment::GetOffPistolAttachPoint()
{
	return OffPistolAttachPoint;
}

FName UArenaCharacterEquipment::GetWristOneAttachPoint()
{
	return WristOneAttachPoint;
}

/////////////////////////////////////////////// Server ///////////////////////////////////////////////

void UArenaCharacterEquipment::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UArenaCharacterEquipment, PrimaryWeapon);
	DOREPLIFETIME(UArenaCharacterEquipment, SecondaryWeapon);
}

void UArenaCharacterEquipment::OnRep_PrimaryWeapon(AArenaWeapon* Weapon)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	PrimaryWeapon = Weapon;
	PrimaryWeapon->SetOwningPawn(MyPawn);
	PrimaryWeapon->SetPrimary(true);
	PrimaryWeapon->UnEquip();
}

void UArenaCharacterEquipment::OnRep_SecondaryWeapon(AArenaWeapon* Weapon)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	SecondaryWeapon = Weapon;
	SecondaryWeapon->SetOwningPawn(MyPawn);
	SecondaryWeapon->SetPrimary(false);
	SecondaryWeapon->UnEquip();
}