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

FString UArenaCharacterEquipment::GetCurrentWeapon()
{
	return CurrentWeapon;
}
void UArenaCharacterEquipment::SetCurrentWeapon(FString Value)
{
	CurrentWeapon = Value;
}

TSubclassOf<class AArenaWeapon> UArenaCharacterEquipment::GetPrimaryWeaponBP()
{
	return PrimaryWeaponBP;
}
void UArenaCharacterEquipment::SetPrimaryWeaponBP(TSubclassOf<class AArenaWeapon> Weapon)
{
	PrimaryWeaponBP = Weapon;
}

TSubclassOf<class AArenaWeapon> UArenaCharacterEquipment::GetSecondaryWeaponBP()
{
	return SecondaryWeaponBP;
}
void UArenaCharacterEquipment::SetSecondaryWeaponBP(TSubclassOf<class AArenaWeapon> Weapon)
{
	SecondaryWeaponBP = Weapon;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetHeadUtilityBP()
{
	return HeadUtilityBP;
}
void UArenaCharacterEquipment::SetHeadUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	HeadUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetUpperBackUtilityBP()
{
	return UpperBackUtilityBP;
}
void UArenaCharacterEquipment::SetUpperBackUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	UpperBackUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetLowerBackUtilityBP()
{
	return LowerBackUtilityBP;
}
void UArenaCharacterEquipment::SetLowerBackUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	LowerBackUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetLeftWristUtilityBP()
{
	return LeftWristUtilityBP;
}
void UArenaCharacterEquipment::SetLeftWristUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	LeftWristUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetRightWristUtilityBP()
{
	return RightWristUtilityBP;
}
void UArenaCharacterEquipment::SetRightWristUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	RightWristUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetLeftWaistUtilityBP()
{
	return LeftWaistUtilityBP;
}
void UArenaCharacterEquipment::SetLeftWaistUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	LeftWaistUtilityBP = Utility;
}

TSubclassOf<class AArenaUtility> UArenaCharacterEquipment::GetRightWaistUtilityBP()
{
	return RightWaistUtilityBP;
}
void UArenaCharacterEquipment::SetRightWaistUtilityBP(TSubclassOf<class AArenaUtility> Utility)
{
	RightWaistUtilityBP = Utility;
}


bool UArenaCharacterEquipment::GetDrawCrosshair()
{
	return DrawCrosshair;
}
void UArenaCharacterEquipment::SetDrawCrosshair(bool Allow)
{
	DrawCrosshair = Allow;
}


TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetChestArmorBP()
{
	return ChestArmorBP;
}
void UArenaCharacterEquipment::SetChestArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	ChestArmorBP = Armor;
}

TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetFeetArmorBP()
{
	return FeetArmorBP;
}
void UArenaCharacterEquipment::SetFeetArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	FeetArmorBP = Armor;
}

TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetHandsArmorBP()
{
	return HandArmorBP;
}
void UArenaCharacterEquipment::SetHandsArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	HandArmorBP = Armor;
}

TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetHeadArmorBP()
{
	return HeadArmorBP;
}
void UArenaCharacterEquipment::SetHeadArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	HeadArmorBP = Armor;
}

TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetLegsArmorBP()
{
	return LegArmorBP;
}
void UArenaCharacterEquipment::SetLegsArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	LegArmorBP = Armor;
}

TSubclassOf<class AArenaArmor> UArenaCharacterEquipment::GetShoulderArmorBP()
{
	return ShoulderArmorBP;
}
void UArenaCharacterEquipment::SetShoulderArmorBP(TSubclassOf<class AArenaArmor> Armor)
{
	ShoulderArmorBP = Armor;
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
	
	DOREPLIFETIME(UArenaCharacterEquipment, CurrentWeapon);
	DOREPLIFETIME(UArenaCharacterEquipment, PrimaryWeapon);
	DOREPLIFETIME(UArenaCharacterEquipment, PrimaryWeaponBP);
	DOREPLIFETIME(UArenaCharacterEquipment, SecondaryWeapon);
	DOREPLIFETIME(UArenaCharacterEquipment, SecondaryWeaponBP);
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