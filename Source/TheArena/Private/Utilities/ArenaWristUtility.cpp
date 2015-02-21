// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaWristUtility.h"

void AArenaWristUtility::StartActivate()
{
	if (Role < ROLE_Authority)
	{
		//ServerStartFire();
	}

	if (!bWantsToActivate)
	{
		bWantsToActivate = true;
	}
}

void AArenaWristUtility::StopActivate()
{
	if (Role < ROLE_Authority)
	{
		//ServerStopFire();
	}

	if (bWantsToActivate)
	{
		bWantsToActivate = false;
	}
}

bool AArenaWristUtility::IsAttachedToPawn() const
{
	return true;
}

void AArenaWristUtility::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWristOneAttachPoint();
		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetPawnMesh();
			Mesh3P->SetHiddenInGame(false);
			Mesh3P->AttachTo(PawnMesh3p, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetUtilityMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachTo(UsePawnMesh, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}

USkeletalMeshComponent* AArenaWristUtility::GetUtilityMesh() const
{
	return Mesh3P;
}

void AArenaWristUtility::DetachMeshFromPawn()
{
	Mesh3P->DetachFromParent();
	Mesh3P->SetHiddenInGame(true);
}

FVector AArenaWristUtility::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetUtilityMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AArenaWristUtility::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetUtilityMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AArenaWristUtility::OnRep_MyPawn()
{
	if (MyPawn)
	{
		//OnEnterInventory(MyPawn);
	}
	else
	{
		//OnLeaveInventory();
	}
}

void AArenaWristUtility::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArenaWristUtility, MyPawn);
}


