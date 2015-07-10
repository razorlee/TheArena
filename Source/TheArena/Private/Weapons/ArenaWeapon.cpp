// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaWeapon.h"

AArenaWeapon::AArenaWeapon(const class FObjectInitializer& PCIP)
{
	//PrimaryComponentTick.bCanEverTick = true;

	Mesh3P = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
}

void AArenaWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RotateWeapon();
	DetachMeshFromPawn();
}

void AArenaWeapon::Destroyed()
{
	Super::Destroyed();

	StopAttack();
}

////////////////////////////////////////// Input handlers //////////////////////////////////////////

void AArenaWeapon::StartAttack()
{

}

void AArenaWeapon::StopAttack()
{

}

void AArenaWeapon::StartReload()
{

}

void AArenaWeapon::StopReload()
{

}

void AArenaWeapon::StartMelee()
{

}

void AArenaWeapon::StopMelee()
{

}

void AArenaWeapon::Equip()
{
	if (ArenaWeaponCan::Equip(MyPawn, this))
	{
		RotateWeapon();
		GetWeaponState()->SetWeaponState(EWeaponState::Equipping);
		float Duration = PlayWeaponAnimation(EquipAnim);

		GetWorldTimerManager().SetTimer(this, &AArenaWeapon::FinishEquip, (Duration*0.25f), false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(EquipSound);
		}
	}
}

void AArenaWeapon::FinishEquip()
{
	DetachMeshFromPawn();
	AttachMeshToPawn();
	GetWeaponState()->SetWeaponState(EWeaponState::Default);
}

float AArenaWeapon::UnEquip()
{
	float Duration = 0;
	if (ArenaWeaponCan::UnEquip(MyPawn, this))
	{
		RotateWeapon();
		StopAttack();
		GetWeaponState()->SetWeaponState(EWeaponState::Holstering);

		Duration = PlayWeaponAnimation(UnEquipAnim);
		//if (WeaponState->GetWeaponState() == EWeaponState::Reloading)
		//{
		//	StopWeaponAnimation(ReloadAnim);
		GetWorldTimerManager().SetTimer(this, &AArenaWeapon::FinishUnEquip, (Duration*0.5f), false);
		//	GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::StopReload);
		//	GetWorldTimerManager().ClearTimer(this, &AArenaRangedWeapon::ReloadWeapon);

		//	WeaponState->SetWeaponState(EWeaponState::Default);
		//}
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(UnEquipSound);
		}
		return Duration;
	}
	return Duration;
}

void AArenaWeapon::FinishUnEquip()
{
	FName AttachPoint;
	if (WeaponClass == EWeaponClass::HeavyWeapon)
	{
		AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainHeavyAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffHeavyAttachPoint();
	}
	else if (WeaponClass == EWeaponClass::Pistol)
	{
		AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainPistolAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffPistolAttachPoint();
	}
	else
	{
		AttachPoint = IsPrimary() ? MyPawn->GetCharacterEquipment()->GetMainWeaponAttachPoint() : MyPawn->GetCharacterEquipment()->GetOffWeaponAttachPoint();
	}

	USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
	Mesh3P->SetHiddenInGame(false);
	Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
	GetWeaponState()->SetWeaponState(EWeaponState::Default);
}

////////////////////////////////////////// Sound Controls //////////////////////////////////////////

UAudioComponent* AArenaWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::PlaySoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

//////////////////////////////////////// Animation Controls ////////////////////////////////////////

float AArenaWeapon::PlayWeaponAnimation(class UAnimMontage* Animation, float InPlayRate)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim, InPlayRate);
		}
	}

	return Duration;
}

void AArenaWeapon::StopWeaponAnimation(class UAnimMontage* Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

///////////////////////////////////////// Socket Controls /////////////////////////////////////////

void AArenaWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetCharacterEquipment()->GetWeaponAttachPoint();
		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
			Mesh3P->SetHiddenInGame(false);
			Mesh3P->AttachTo(PawnMesh3p, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachTo(UsePawnMesh, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}

void AArenaWeapon::DetachMeshFromPawn()
{
	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
}

/////////////////////////////////////// Getters and Setters ///////////////////////////////////////

class AArenaCharacter* AArenaWeapon::GetPawnOwner() const
{
	return MyPawn;
}
void AArenaWeapon::SetOwningPawn(AArenaCharacter* Character)
{
	Instigator = Character;
	MyPawn = Character;
	SetOwner(Character);
}

EWeaponClass::Type AArenaWeapon::GetWeaponClass()
{
	return WeaponClass;
}
void AArenaWeapon::SetWeaponClass(EWeaponClass::Type NewClass)
{
	WeaponClass = NewClass;
}

bool AArenaWeapon::IsPrimary()
{
	return PrimaryWeapon;
}
void AArenaWeapon::SetPrimary(bool Status)
{
	PrimaryWeapon = Status;
}

FName AArenaWeapon::GetWeaponName() const
{
	return WeaponName;
}

USkeletalMeshComponent* AArenaWeapon::GetWeaponMesh() const
{
	return Mesh3P;
}

class UArenaRangedWeaponState* AArenaWeapon::GetWeaponState()
{
	return NULL;
}

class UArenaRangedWeaponAttributes* AArenaWeapon::GetWeaponAttributes()
{
	return NULL;
}