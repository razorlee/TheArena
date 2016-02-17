// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaHealthPickup.h"


// Sets default values
AArenaHealthPickup::AArenaHealthPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AArenaHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaHealthPickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

