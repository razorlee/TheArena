// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaEquipmentLocker.generated.h"

UCLASS()
class THEARENA_API AArenaEquipmentLocker : public AActor
{
	GENERATED_BODY()
	
public:	

	AArenaEquipmentLocker(const class FObjectInitializer& PCIP);

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = Trigger)
	UBoxComponent* Trigger;
	
	
};
