// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterInventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaCharacterInventory : public UActorComponent
{
	GENERATED_BODY()

public:	

	UArenaCharacterInventory();

	virtual void InitializeComponent() override;
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void SpawnDefaultInventory();

	UFUNCTION(BlueprintCallable, Category = Inventory)
	TArray<class AArenaWeapon*> GetInventory();

protected:

	UPROPERTY(EditAnywhere, Category = Inventory)
	TArray<TSubclassOf<class AArenaWeapon>> DefaultInventoryClasses;

	UPROPERTY()
	TArray<class AArenaWeapon*> Inventory;
};
