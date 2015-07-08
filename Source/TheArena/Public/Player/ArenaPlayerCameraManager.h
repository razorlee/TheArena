#pragma once

#include "Camera/PlayerCameraManager.h"
#include "Player/ArenaCharacterState.h"
#include "Weapons/ArenaRangedWeaponState.h"
#include "ArenaPlayerCameraManager.generated.h"

UCLASS()
class THEARENA_API AArenaPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	virtual void UpdateCamera(float DeltaTime) override;

	void UpdateCurrents(float DeltaTime);

	void HandlePassiveCamera();

	void HandleAggressiveCamera();

	void HandleTargetingCamera();

	void HandleRunningCamera();

	void HandleCoverCamera();

	void HandleLowCoverCamera(ECoverState::Type CoverState);

	void HandleHighCoverCamera(ECoverState::Type CoverState);

	void HandleFaceLeftCamera(FString State, ETargetingState::Type TargetingState, ECoverState::Type CoverState);

	void HandleFaceRightCamera(FString State, ETargetingState::Type TargetingState, ECoverState::Type CoverState);

private:

	UPROPERTY()
	AArenaCharacter* MyPawn;

	UPROPERTY()
	float Speed;

	UPROPERTY()
	float CurrentArm;
	UPROPERTY()
	float TargetArm;

	UPROPERTY()
	FVector CurrentOffset;
	UPROPERTY()
	FVector TargetOffset;



};
