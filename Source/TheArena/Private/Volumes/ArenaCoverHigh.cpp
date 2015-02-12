// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCoverHigh.h"

AArenaCoverHigh::AArenaCoverHigh(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Cover = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Cover"));
	Cover->bGenerateOverlapEvents = true;
	Cover->SetRelativeScale3D(FVector(2, 0.5, 3));
	RootComponent = Cover;

	//Cover->OnComponentBeginOverlap.AddDynamic(this, &AArenaCoverHigh::TriggerEnter);
	//Cover->OnComponentEndOverlap.AddDynamic(this, &AArenaCoverHigh::TriggerExit);
}


void AArenaCoverHigh::TriggerEnter(class AActor* OtherActor)
{

}

void AArenaCoverHigh::TriggerExit(class AActor* OtherActor)
{

}
