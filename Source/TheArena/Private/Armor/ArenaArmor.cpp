// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaArmor.h"


// Sets default values
AArenaArmor::AArenaArmor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

