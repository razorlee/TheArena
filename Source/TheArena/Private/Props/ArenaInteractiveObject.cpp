// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaInteractiveObject.h"


// Sets default values
AArenaInteractiveObject::AArenaInteractiveObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	InteractText = FText::FromString(FString("Interact"));

	// Defaults to active
	bActive = true;
}

// Called when the game starts or when spawned
void AArenaInteractiveObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AArenaInteractiveObject::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}


void AArenaInteractiveObject::OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	// Play animation
	UAnimMontage* UseAnim = InteractAnimation;
	if (Player && UseAnim)
	{
		float Duration = Player->PlayAnimMontage(UseAnim, InPlayRate);

		// Debug Message
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Playing animation %d"), Duration));
	}
}

void AArenaInteractiveObject::OnView_Implementation(AArenaCharacter* Player)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	// Show interact message
	/*
	AArenaPlayerController* PlayerController = Cast<AArenaPlayerController>(Player->GetController());
	if (PlayerController)
	{
		PlayerController->SetInteractiveMessage(InteractText);
	}
	*/
}

void AArenaInteractiveObject::OnLeave_Implementation(AArenaCharacter* Player)
{
	// Hide interact message
	/*
	AArenaPlayerController* PlayerController = Cast<AArenaPlayerController>(Player->GetController());
	if (PlayerController)
	{
		PlayerController->SetInteractiveMessage(FText());
	}
	*/
}


bool AArenaInteractiveObject::IsActive()
{
	return bActive;
}

void AArenaInteractiveObject::SetActive(bool isActive)
{
	bActive = isActive;
}

FText AArenaInteractiveObject::GetInteractText()
{
	return InteractText;
}

void AArenaInteractiveObject::SetInteractText(FText newText)
{
	InteractText = newText;
}

UAnimMontage* AArenaInteractiveObject::GetInteractAnimation()
{
	return InteractAnimation;
}

void AArenaInteractiveObject::SetInteractAnimation(UAnimMontage* newAnim)
{
	InteractAnimation = newAnim;
}