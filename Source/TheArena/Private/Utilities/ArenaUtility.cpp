// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaUtility.h"

// Sets default values
AArenaUtility::AArenaUtility()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Active = false;
}

// Called when the game starts or when spawned
void AArenaUtility::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaUtility::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Active && (ActivationType == EActivationType::Toggle || ActivationType == EActivationType::Channel))
	{
		ConsumeEnergy(DeltaTime);
		if (MyPawn->GetCharacterAttributes()->GetCurrentEnergy() <= 0.0f)
		{
			Deactivate();
		}
	}
}

class AArenaCharacter* AArenaUtility::GetMyPawn() const
{
	return MyPawn;
}
void AArenaUtility::SetMyPawn(AArenaCharacter* Pawn)
{
	MyPawn = Pawn;
}

FName AArenaUtility::GetUtilityName() const
{
	return UtilityName;
}

float AArenaUtility::GetActivationCost() const
{
	return ActivationCost;
}
void AArenaUtility::SetActivationCost(float Value)
{
	ActivationCost = Value;
}

void AArenaUtility::Equip()
{
	FName AttachPoint;
	if (UtilityType == EUtilityType::Head)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainHeavyAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffHeavyAttachPoint();
	}
	else if (UtilityType == EUtilityType::UpperBack)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainPistolAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffPistolAttachPoint();
	}
	else if (UtilityType == EUtilityType::LowerBack)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (UtilityType == EUtilityType::Wrist)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (UtilityType == EUtilityType::Waist)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}

	if (MyPawn->IsLocallyControlled() == true)
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
	else
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
}
void AArenaUtility::UnEquip()
{
}

EUtilityType::Type AArenaUtility::GetUtilityType()
{
	return UtilityType;
}

EActivationType::Type AArenaUtility::GetActivationType()
{
	return ActivationType;
}

void AArenaUtility::Activate()
{
	ConsumeEnergy();
	ActivateBP();
	// Do nothing here
}
void AArenaUtility::Deactivate()
{
	DeactivateBP();
	// Do nothing here
}

void AArenaUtility::ConsumeEnergy(float DeltaSeconds)
{
	MyPawn->GetCharacterAttributes()->SetCurrentEnergy(MyPawn->GetCharacterAttributes()->GetCurrentEnergy() - (ActivationCost * DeltaSeconds));
}