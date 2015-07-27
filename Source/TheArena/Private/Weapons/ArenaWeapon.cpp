// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaWeapon.h"

AArenaWeapon::AArenaWeapon(const class FObjectInitializer& PCIP)
{
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

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	//bReplicateInstigator = true;
	bNetUseOwnerRelevancy = true;
}

void AArenaWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	DetachMeshFromPawn();
}

void AArenaWeapon::Destroyed()
{
	Super::Destroyed();
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

void AArenaWeapon::StartMelee(bool bFromReplication)
{
	if (ArenaWeaponCan::Melee(MyPawn, this))
	{
		if (Role == ROLE_Authority)
		{
			Melee();
		}
		else
		{
			ServerMelee();
		}
	}
}

void AArenaWeapon::StopMelee()
{
	if (GetWeaponState()->GetWeaponState() == EWeaponState::Meleeing)
	{
		GetWeaponState()->SetWeaponState(EWeaponState::Default);
		StopWeaponAnimation(GetWeaponEffects()->GetMeleeAnim());
	}
}

void AArenaWeapon::Equip()
{
	if (ArenaWeaponCan::Equip(MyPawn, this))
	{
		GetWeaponState()->SetWeaponState(EWeaponState::Equipping);
		GetWeaponState()->SetEquippedState(EEquippedState::Equipped);
		float Duration = PlayWeaponAnimation(EquipAnim, GetWeaponAttributes()->GetMotility()) / GetWeaponAttributes()->GetMotility();

		GetWorldTimerManager().SetTimer(this, &AArenaWeapon::FinishEquip, (Duration*0.25f), false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(EquipSound);
		}
	}
}

float AArenaWeapon::UnEquip()
{
	float Duration = 0;
	if (ArenaWeaponCan::UnEquip(MyPawn, this))
	{
		StopAttack();

		Duration = PlayWeaponAnimation(UnEquipAnim, GetWeaponAttributes()->GetMotility()) / GetWeaponAttributes()->GetMotility();
		if (GetWeaponState()->GetWeaponState() == EWeaponState::Reloading)
		{
			StopWeaponAnimation(GetWeaponEffects()->GetReloadAnim());
			
			GetWorldTimerManager().ClearTimer(Reload_Timer);
			GetWorldTimerManager().ClearTimer(StopReload_Timer);
		}

		GetWeaponState()->SetWeaponState(EWeaponState::Holstering);
		GetWeaponState()->SetEquippedState(EEquippedState::UnEquipped);
		GetWorldTimerManager().SetTimer(this, &AArenaWeapon::FinishUnEquip, (Duration*0.5f), false);
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(UnEquipSound);
		}
		return Duration;
	}
	return Duration;
}

/////////////////////////////////////// Input Implementation ///////////////////////////////////////

void AArenaWeapon::OnEnterInventory(AArenaCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AArenaWeapon::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}

	if (GetWeaponState()->GetEquippedState() == EEquippedState::Equipped)
	{
		UnEquip();
	}
}

void AArenaWeapon::Melee_Implementation()
{
	StopAttack();
	GetWeaponState()->SetWeaponState(EWeaponState::Meleeing);
	float AnimDuration = PlayWeaponAnimation(GetWeaponEffects()->GetMeleeAnim());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 0.3f;
	}

	GetWorldTimerManager().SetTimer(this, &AArenaWeapon::StopMelee, AnimDuration, false);
	if (Role == ROLE_Authority)
	{
		TArray<struct FOverlapResult> OutOverlaps;
		TArray<AActor*> HitActors;

		FQuat Rotation = Instigator->GetTransform().GetRotation();
		FVector Start = Instigator->GetTransform().GetLocation() + Rotation.Rotator().Vector() * 100.0f;

		FCollisionShape CollisionHitShape;
		FCollisionQueryParams CollisionParams;

		CollisionParams.AddIgnoredActor(Instigator);

		FCollisionObjectQueryParams CollisionObjectTypes;
		CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
		CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

		CollisionHitShape = FCollisionShape::MakeBox(FVector(60.0f, 60.0f, 0.5f));
		GetWorld()->OverlapMulti(OutOverlaps, Start, Rotation, CollisionHitShape, CollisionParams, CollisionObjectTypes);

		for (int i = 0; i < OutOverlaps.Num(); ++i)
		{
			if (OutOverlaps[i].GetActor() && !HitActors.Contains(OutOverlaps[i].GetActor()))
			{
				if (!OutOverlaps[i].GetActor() || HitActors.Contains(OutOverlaps[i].GetActor()))
				{
					return;
				}
				HitActors.AddUnique(OutOverlaps[i].GetActor());
				FHitResult AttackHitResult;
				const FDamageEvent AttackDamageEvent;
				AArenaCharacter* GameCharacter = Cast<AArenaCharacter>(OutOverlaps[i].GetActor());

				if (GameCharacter)
				{
					UGameplayStatics::ApplyDamage(OutOverlaps[i].GetActor(), 200.0f, Instigator->GetController(), MyPawn->Controller, UDamageType::StaticClass());
				}
			}
			OutOverlaps.Empty();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(GetWeaponEffects()->GetMeleeSound());
	}
}

void AArenaWeapon::FinishEquip()
{
	DetachMeshFromPawn();
	AttachMeshToPawn();
	GetWeaponState()->SetWeaponState(EWeaponState::Default);
}

void AArenaWeapon::FinishUnEquip()
{
	FName AttachPoint;
	DetachMeshFromPawn();
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

	if (MyPawn->IsLocallyControlled() == true)
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
		GetWeaponState()->SetWeaponState(EWeaponState::Default);
	}
	else
	{
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
		Mesh3P->SetHiddenInGame(false);
		Mesh3P->AttachTo(PawnMesh3p, AttachPoint, EAttachLocation::SnapToTarget, true);
		GetWeaponState()->SetWeaponState(EWeaponState::Default);
	}


	/*USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
	USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
	UseWeaponMesh->AttachTo(UsePawnMesh, AttachPoint);
	UseWeaponMesh->SetHiddenInGame(false);*/
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
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
			Mesh3P->SetHiddenInGame(false);
			Mesh3P->AttachTo(PawnMesh3p, AttachPoint);
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

class UArenaRangedWeaponEffects* AArenaWeapon::GetWeaponEffects()
{
	return NULL;
}

///////////////////////////////////////////// Server /////////////////////////////////////////////

void AArenaWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaWeapon, MyPawn);
}

void AArenaWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

bool AArenaWeapon::ServerMelee_Validate()
{
	return true;
}
void AArenaWeapon::ServerMelee_Implementation()
{
	Melee();
}