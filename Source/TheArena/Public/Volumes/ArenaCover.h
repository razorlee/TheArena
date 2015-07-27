// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaCover.generated.h"

UENUM()
namespace ECoverType
{
	enum Type
	{
		Both	UMETA(DisplayName = "Both"),
		Right	UMETA(DisplayName = "Right Only"),
		Left	UMETA(DisplayName = "Left Only")
	};
}

UCLASS()
class THEARENA_API AArenaCover : public AActor
{
	GENERATED_BODY()

public:

	AArenaCover(const class FObjectInitializer& PCIP);

	UPROPERTY(EditAnywhere)
	TSubobjectPtr<UBoxComponent>Cover;

	UPROPERTY(EditAnywhere)
	TSubobjectPtr<UBoxComponent>LeftEdge;

	UPROPERTY(EditAnywhere)
	TSubobjectPtr<UBoxComponent>RightEdge;

	UPROPERTY(EditAnywhere)
	class UArrowComponent* Arrow;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION() 
	void BeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void BeginLeftOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndLeftOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void BeginRightOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void EndRightOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleCoverState(class AArenaCharacter* MyPawn);

protected:

	UPROPERTY(EditAnywhere, Category = Cover)
	TArray<AArenaCharacter*> CoverActors;

	UPROPERTY(EditAnywhere, Category = Cover)
	TEnumAsByte<ECoverType::Type> CoverType;

	UPROPERTY(EditAnywhere, Category = Cover)
	bool IsLowCover;

	UPROPERTY(EditAnywhere, Category = Cover)
	bool CanVaultOver;

	UPROPERTY(EditAnywhere, Category = Cover)
	bool CanClimbUp;

};
