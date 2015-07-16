// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaCover.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API AArenaCover : public AActor
{
	GENERATED_BODY()

public:

	AArenaCover(const class FObjectInitializer& PCIP);

	UPROPERTY()
	TSubobjectPtr<UBoxComponent>Cover;

	UFUNCTION() 
	void BeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:

	UPROPERTY(EditDefaultsOnly, Category = Cover)
	bool IsLowCover;

	UPROPERTY(EditDefaultsOnly, Category = Cover)
	bool CanVaultOver;

	UPROPERTY(EditDefaultsOnly, Category = Cover)
	bool CanClimbUp;

};
