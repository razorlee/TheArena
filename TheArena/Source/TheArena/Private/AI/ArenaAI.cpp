// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"

AArenaAI::AArenaAI(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	AIControllerClass = AArenaAIController::StaticClass();

	UpdatePawnMeshes();

	bUseControllerRotationYaw = true;
}

void AArenaAI::FaceRotation(FRotator NewRotation, float DeltaTime)
{
	FRotator CurrentRotation = FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 8.0f);

	Super::FaceRotation(CurrentRotation, DeltaTime);
}
