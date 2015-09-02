// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaUtility.h"

// Sets default values
AArenaUtility::AArenaUtility(const class FObjectInitializer& PCIP)
{
	Mesh3P = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("UtilityMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	Active = false;
}

void AArenaUtility::Destroyed()
{
	Super::Destroyed();
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
		ConsumeEnergy(ContinuationCost, DeltaTime);
		if (MyPawn->GetCharacterAttributes()->GetCurrentEnergy() <= 0.0f)
		{
			Deactivate();
			Active = false;
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
	SetOwner(Pawn);
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
		AttachPoint = FName(TEXT("UpperBackUtility"));
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
	if (Role == ROLE_Authority)
	{
		SetMyPawn(NULL);
	}

	Deactivate();
	Active = false;
	
	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
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
	if (Role == ROLE_Authority)
	{
		if (!Active)
		{
			ConsumeEnergy(ActivationCost);
			PlayUtilityAnimation();
		}
		ActivateBP();
		// Do nothing here
	}
	else
	{
		ServerActivate();
	}
}
void AArenaUtility::Deactivate()
{
	if (Role == ROLE_Authority)
	{
		DeactivateBP();
		// Do nothing here
	}
	else
	{
		ServerDeactivate();
	}
}

void AArenaUtility::ConsumeEnergy(float DeltaSeconds, float Cost)
{
	MyPawn->GetCharacterAttributes()->SetCurrentEnergy(MyPawn->GetCharacterAttributes()->GetCurrentEnergy() - (Cost * DeltaSeconds));
}

void AArenaUtility::PlayUtilityAnimation_Implementation()
{
	if (UtilityAnim)
	{
		MyPawn->PlayAnimMontage(UtilityAnim);
	}
}

/////////////////////////////////////// Server ///////////////////////////////////////

void AArenaUtility::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaUtility, MyPawn);
	DOREPLIFETIME(AArenaUtility, Active);
}

void AArenaUtility::OnRep_MyPawn()
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

bool AArenaUtility::ServerActivate_Validate()
{
	return true;
}
void AArenaUtility::ServerActivate_Implementation()
{
	if (!Active)
	{
		ConsumeEnergy(ActivationCost);
		PlayUtilityAnimation();
	}
	ActivateBP();
}

bool AArenaUtility::ServerDeactivate_Validate()
{
	return true;
}
void AArenaUtility::ServerDeactivate_Implementation()
{
	DeactivateBP();
}