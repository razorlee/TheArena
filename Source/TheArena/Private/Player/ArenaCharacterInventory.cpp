// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCharacterInventory.h"


// Sets default values for this component's properties
UArenaCharacterInventory::UArenaCharacterInventory()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UArenaCharacterInventory::InitializeComponent()
{
	Super::InitializeComponent();

	// ...
	
}


// Called every frame
void UArenaCharacterInventory::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

