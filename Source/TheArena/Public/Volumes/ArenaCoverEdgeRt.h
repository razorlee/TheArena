// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaCoverEdgeRt.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaCoverEdgeRt : public AActor
{
	GENERATED_BODY()
	
	AArenaCoverEdgeRt(const class FObjectInitializer& PCIP);

	TSubobjectPtr<UBoxComponent>Cover;

	UFUNCTION()
	void TriggerEnter(class AActor* OtherActor);

	UFUNCTION()
	void TriggerExit(class AActor* OtherActor);
	
};
