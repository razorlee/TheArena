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
	}

	// Log debug message
	FString interactDebugString = GetName();
	UE_LOG(LogClass, Log, TEXT("You have interacted with %s"), *interactDebugString);
}

void AArenaInteractiveObject::OnView_Implementation(AArenaCharacter* Player)
{
	// Can only interact if active
	if (!IsActive()) {
		return;
	}

	// Show interact message
	AArenaPlayerController* PlayerController = Cast<AArenaPlayerController>(Player->GetController());
	if (PlayerController)
	{
		PlayerController->SetInteractiveMessage(InteractText);
	}

	// Log debug message
	FString interactDebugString = GetName();
	UE_LOG(LogClass, Log, TEXT("You have viewed %s"), *interactDebugString);
}

void AArenaInteractiveObject::OnLeave_Implementation(AArenaCharacter* Player)
{
	// Hide interact message
	AArenaPlayerController* PlayerController = Cast<AArenaPlayerController>(Player->GetController());
	if (PlayerController)
	{
		PlayerController->SetInteractiveMessage(FText());
	}

	// Log debug message
	FString interactDebugString = GetName();
	UE_LOG(LogClass, Log, TEXT("You have stopped viewing %s"), *interactDebugString);
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