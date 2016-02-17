// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaHealthPickup.generated.h"

UCLASS()
class THEARENA_API AArenaHealthPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArenaHealthPickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere, Category = Properties)
	float HealAmount;

	
	
};
