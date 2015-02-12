// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaCoverEdgeRt.h"

AArenaCoverEdgeRt::AArenaCoverEdgeRt(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;

	Cover = PCIP.CreateDefaultSubobject<UBoxComponent>(this, TEXT("Cover"));
	Cover->bGenerateOverlapEvents = true;
	Cover->SetRelativeScale3D(FVector(0.5, 0.5, 3));
	RootComponent = Cover;

	//Cover->OnComponentBeginOverlap.AddDynamic(this, &AArenaCoverHigh::TriggerEnter);
	//Cover->OnComponentEndOverlap.AddDynamic(this, &AArenaCoverHigh::TriggerExit);
}


void AArenaCoverEdgeRt::TriggerEnter(class AActor* OtherActor)
{

}

void AArenaCoverEdgeRt::TriggerExit(class AActor* OtherActor)
{

}

