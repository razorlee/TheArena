// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCover.h"

AArenaCover::AArenaCover(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Cover = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Cover"));
	Cover->bGenerateOverlapEvents = true;
	Cover->SetRelativeScale3D(FVector(2, 1, 3));
	RootComponent = Cover;

	//Cover->OnComponentBeginOverlap.AddDynamic(this, &AArenaCoverLow::TriggerEnter);
	//Cover->OnComponentEndOverlap.AddDynamic(this, &AArenaCoverLow::TriggerExit);
}


void AArenaCover::TriggerEnter(class AActor* OtherActor)
{

}

void AArenaCover::TriggerExit(class AActor* OtherActor)
{

}


