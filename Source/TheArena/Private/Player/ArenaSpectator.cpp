// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaSpectator.h"

AArenaSpectator::AArenaSpectator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bUseControllerRotationPitch = true;
	this->bUseControllerRotationRoll = true;
	this->bUseControllerRotationYaw = true;
	index = 0;
}

void AArenaSpectator::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Spectating")));

	OriginalController = this->Controller;
}

void AArenaSpectator::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAction("NextCamera", IE_Pressed, this, &AArenaSpectator::OnNextCamera);
	InputComponent->BindAction("PreviousCamera", IE_Pressed, this, &AArenaSpectator::OnPreviousCamera);
	/*InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AArenaSpectator::LookUpAtRate);*/
}

void AArenaSpectator::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AArenaSpectator::OnNextCamera()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Next")));
	AArenaGameState* const GameState = GetWorld() != NULL ? GetWorld()->GetGameState<AArenaGameState>() : NULL;
	if (ensure(GameState != nullptr))
	{
		index = index >= (GameState->PlayerArray.Num() - 1) ? 0 : (index + 1);

		AArenaPlayerState* FollowPlayerState = GameState->PlayerArray[index] != NULL ? Cast<AArenaPlayerState>(GameState->PlayerArray[index]) : NULL;
		AArenaPlayerState* const MyPlayerState = Cast<AArenaPlayerState>(this->Controller->PlayerState);

		while (FollowPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
		{
			index = index >= (GameState->PlayerArray.Num() - 1) ? 0 : (index + 1);
			FollowPlayerState = GameState->PlayerArray[index] != NULL ? Cast<AArenaPlayerState>(GameState->PlayerArray[index]) : NULL;
		}

		FollowPawn = Cast<AArenaCharacter>(FollowPlayerState->MyPawn);

		if (FollowPlayerState != NULL && FollowPawn->GetController())
		{
			this->Controller = FollowPawn->GetController();
			this->RootComponent->AttachTo(FollowPawn->CameraBoom, USpringArmComponent::SocketName, EAttachLocation::SnapToTarget, true);
		}
	}
}

void AArenaSpectator::OnPreviousCamera()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Previous")));
	AArenaGameState* const GameState = GetWorld() != NULL ? GetWorld()->GetGameState<AArenaGameState>() : NULL;
	if (ensure(GameState != nullptr))
	{
		index = index <= 0 ? (GameState->PlayerArray.Num() - 1) : (index - 1);

		AArenaPlayerState* FollowPlayerState = GameState->PlayerArray[index] != NULL ? Cast<AArenaPlayerState>(GameState->PlayerArray[index]) : NULL;
		AArenaPlayerState* const MyPlayerState = Cast<AArenaPlayerState>(this->Controller->PlayerState);

		while (FollowPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
		{
			index = index <= 0 ? (GameState->PlayerArray.Num() - 1) : (index - 1);
			FollowPlayerState = GameState->PlayerArray[index] != NULL ? Cast<AArenaPlayerState>(GameState->PlayerArray[index]) : NULL;
		}

		FollowPawn = Cast<AArenaCharacter>(FollowPlayerState->MyPawn);

		if (FollowPlayerState != NULL && FollowPawn->GetController())
		{
			this->Controller = FollowPawn->GetController();
			this->RootComponent->AttachTo(FollowPawn->CameraBoom, USpringArmComponent::SocketName, EAttachLocation::SnapToTarget, true);
		}
	}
}

