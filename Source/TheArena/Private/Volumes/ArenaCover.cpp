// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCover.h"

AArenaCover::AArenaCover(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Cover = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Cover"));
	Cover->bGenerateOverlapEvents = true;

	LeftEdge = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Left Edge"));
	LeftEdge->bGenerateOverlapEvents = true;

	RightEdge = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Right Edge"));
	RightEdge->bGenerateOverlapEvents = true;

	Arrow = PCIP.CreateDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));

	RootComponent = Cover;

	Cover->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Cover->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	Cover->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	LeftEdge->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	LeftEdge->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	LeftEdge->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	RightEdge->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RightEdge->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	RightEdge->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	Cover->OnComponentBeginOverlap.AddDynamic(this, &AArenaCover::BeginOverlap);
	Cover->OnComponentEndOverlap.AddDynamic(this, &AArenaCover::EndOverlap);

	LeftEdge->OnComponentBeginOverlap.AddDynamic(this, &AArenaCover::BeginLeftOverlap);
	LeftEdge->OnComponentEndOverlap.AddDynamic(this, &AArenaCover::EndLeftOverlap);

	RightEdge->OnComponentBeginOverlap.AddDynamic(this, &AArenaCover::BeginRightOverlap);
	RightEdge->OnComponentEndOverlap.AddDynamic(this, &AArenaCover::EndRightOverlap);
}

void AArenaCover::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	for (int i = 0; i < CoverActors.Num(); i++)
	{
		AArenaCharacter* MyPawn = CoverActors[i];
		MyPawn->GetPlayerState()->SetIsNearCover(true);
		HandleCoverState(MyPawn);

		FRotator NewRotation = MyPawn->GetActorRotation();
		NewRotation.Yaw = Arrow->GetComponentRotation().Yaw;
		MyPawn->SetActorRotation(NewRotation);

		const FRotator Rotation = Arrow->GetComponentRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		MyPawn->GetPlayerMovement()->SetCoverDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	}
}

void AArenaCover::BeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn)
	{
		CoverActors.AddUnique(MyPawn);
		MyPawn->GetPlayerState()->SetIsNearCover(true);
	}	
}
void AArenaCover::EndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn)
	{
		CoverActors.RemoveSingle(MyPawn);
		MyPawn->GetPlayerState()->SetIsNearCover(false);
	}
}

void AArenaCover::BeginLeftOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn && CoverType != ECoverType::Right && !MyPawn->GetPlayerState()->GetIsNearRightEdge())
	{
		MyPawn->GetPlayerState()->SetIsNearCover(true);
		MyPawn->GetPlayerState()->SetIsNearLeftEdge(true);
	}
}
void AArenaCover::EndLeftOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn && !MyPawn->GetCurrentWeapon()->GetWeaponState()->GetCoverTargeting())
	{
		MyPawn->GetPlayerState()->SetIsNearLeftEdge(false);
	}
}

void AArenaCover::BeginRightOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn && CoverType != ECoverType::Left && !MyPawn->GetPlayerState()->GetIsNearLeftEdge())
	{
		MyPawn->GetPlayerState()->SetIsNearCover(true);
		MyPawn->GetPlayerState()->SetIsNearRightEdge(true);
	}
}
void AArenaCover::EndRightOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn && !MyPawn->GetCurrentWeapon()->GetWeaponState()->GetCoverTargeting())
	{
		MyPawn->GetPlayerState()->SetIsNearRightEdge(false);
	}
}

void AArenaCover::HandleCoverState(class AArenaCharacter* MyPawn)
{
	if (IsLowCover)
	{
		if (MyPawn->GetPlayerState()->GetIsNearRightEdge())
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::LowRight);
		}
		else if (MyPawn->GetPlayerState()->GetIsNearLeftEdge())
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::LowLeft);
		}
		else
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::LowMiddle);
		}
	}
	else
	{
		if (MyPawn->GetPlayerState()->GetIsNearRightEdge())
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::HighRight);
		}
		else if (MyPawn->GetPlayerState()->GetIsNearLeftEdge())
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::HighLeft);
		}
		else
		{
			MyPawn->GetPlayerState()->SetCoverState(ECoverState::HighMiddle);
		}
	}
}
