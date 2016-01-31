// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaArmor.h"


// Sets default values
AArenaArmor::AArenaArmor(const class FObjectInitializer& PCIP)
{
	Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ArmorMesh3P"));
	//Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	//Mesh3P->bChartDistanceFactor = true;
	Mesh->bReceivesDecals = false;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

// Called when the game starts or when spawned
void AArenaArmor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaArmor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

class AArenaCharacter* AArenaArmor::GetMyPawn() const
{
	return MyPawn;
}
void AArenaArmor::SetMyPawn(AArenaCharacter* Pawn)
{
	MyPawn = Pawn;
	SetOwner(Pawn);
}

FString AArenaArmor::GetDescription()
{
	return ArmorDescription;
}
void AArenaArmor::SetDescription(FString Value)
{
	ArmorDescription = Value;
}

FName AArenaArmor::GetArmorName() const
{
	return ArmorName;
}

EArmorType::Type AArenaArmor::GetArmorType()
{
	return ArmorType;
}

float AArenaArmor::GetProtection() const
{
	return ArmorStats.Protection;
}
void AArenaArmor::SetProtection(float Value)
{
	ArmorStats.Protection = Value;
}

float AArenaArmor::GetMotility() const
{
	return ArmorStats.Motility;
}
void AArenaArmor::SetMotility(float Value)
{
	ArmorStats.Motility = Value;
}

void AArenaArmor::Equip()
{
	FName AttachPoint;
	if (ArmorType == EArmorType::Head)
	{
		AttachPoint = FName(TEXT("ArmorHead"));
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainHeavyAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffHeavyAttachPoint();
	}
	else if (ArmorType == EArmorType::Shoulders)
	{
		//AttachPoint = FName(TEXT("UpperBackUtility"));
	}
	else if (ArmorType == EArmorType::Chest)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (ArmorType == EArmorType::Legs)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (ArmorType == EArmorType::Feet)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}
	else if (ArmorType == EArmorType::Hands)
	{
		//AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}

	if (MyPawn->IsLocallyControlled() == true)
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh->SetHiddenInGame(false);
		Mesh->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
	else
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh->SetHiddenInGame(false);
		Mesh->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	}
}
void AArenaArmor::UnEquip()
{
	if (Role == ROLE_Authority)
	{
		SetMyPawn(NULL);
	}
	
	Mesh->DetachFromParent();
	Mesh->SetHiddenInGame(true);
}

void AArenaArmor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaArmor, MyPawn);
}

void AArenaArmor::OnRep_MyPawn()
{
	if (MyPawn)
	{
		SetMyPawn(MyPawn);
	}
	else
	{
		UnEquip();
	}
}