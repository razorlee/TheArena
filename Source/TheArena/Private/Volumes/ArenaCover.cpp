// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCover.h"

AArenaCover::AArenaCover(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Cover = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Cover"));
	Cover->bGenerateOverlapEvents = true;
	Cover->SetRelativeScale3D(FVector(2, 1, 3));

	RootComponent = Cover;

	Cover->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Cover->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	Cover->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	Cover->OnComponentBeginOverlap.AddDynamic(this, &AArenaCover::BeginOverlap);
	Cover->OnComponentEndOverlap.AddDynamic(this, &AArenaCover::EndOverlap);
}


void AArenaCover::BeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn)
	{
		MyPawn->GetPlayerState()->SetIsNearCover(true);
	}
}

void AArenaCover::EndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AArenaCharacter* MyPawn = Cast<AArenaCharacter>(OtherActor);
	if (MyPawn)
	{
		MyPawn->GetPlayerState()->SetIsNearCover(false);
	}
}


