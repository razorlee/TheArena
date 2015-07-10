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

	SpawnDefaultInventory();
	
}


// Called every frame
void UArenaCharacterInventory::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UArenaCharacterInventory::SpawnDefaultInventory()
{
	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.bNoCollisionFail = true;
			AArenaWeapon* NewWeapon = GetWorld()->SpawnActor<AArenaWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			Inventory.AddUnique(NewWeapon);
		}
	}
}

TArray<class AArenaWeapon*> UArenaCharacterInventory::GetInventory()
{
	return Inventory;
}