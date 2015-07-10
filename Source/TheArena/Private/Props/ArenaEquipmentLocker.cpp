// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaEquipmentLocker.h"


// Sets default values
AArenaEquipmentLocker::AArenaEquipmentLocker(const class FObjectInitializer& PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	RootComponent = Mesh;

	Trigger = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Trigger"));
	Trigger->SetHiddenInGame(true);
	Trigger->AddLocalOffset(FVector(140.0, -60.0, 100.0));
	Trigger->SetBoxExtent(FVector(30.0, 40.0, 125.0));
}

// Called when the game starts or when spawned
void AArenaEquipmentLocker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaEquipmentLocker::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

