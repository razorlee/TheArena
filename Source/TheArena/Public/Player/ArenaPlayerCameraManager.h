#pragma once

#include "Camera/PlayerCameraManager.h"
#include "Player/ArenaCharacterState.h"
#include "Weapons/ArenaRangedWeaponState.h"
#include "ArenaPlayerCameraManager.generated.h"

#define LEFT_SHOULDER -1
#define RIGHT_SHOULDER 1

UCLASS()
class THEARENA_API AArenaPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	virtual void UpdateCamera(float DeltaTime) override;

	void UpdateCurrents(float DeltaTime);

	void ToggleShoulder();

	void HandlePassiveCamera();

	void HandleAggressiveCamera();

	void HandleTargetingCamera();

	void HandleRunningCamera();

	void HandleCoverCamera();

	void HandleLowCoverCamera(ECoverState::Type CoverState);

	void HandleHighCoverCamera(ECoverState::Type CoverState);

	void HandleFaceLeftCamera(FString State, ECoverState::Type CoverState);

	void HandleFaceRightCamera(FString State, ECoverState::Type CoverState);

////////////////////////////////////////////////// Getter and Setters //////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Camera)
		float GetNormalFOV();
	UFUNCTION(BlueprintCallable, Category = Camera)
		void SetNormalFOV(float Value);

private:

	UPROPERTY()
	AArenaCharacter* MyPawn;

	UPROPERTY()
	float Speed;

	UPROPERTY()
		float NormalFOV;

	UPROPERTY()
	float CurrentFOV;
	UPROPERTY()
	float TargetFOV;

	UPROPERTY()
	float CurrentArm;
	UPROPERTY()
	float TargetArm;

	UPROPERTY()
	FVector CurrentOffset;
	UPROPERTY()
	FVector TargetOffset;

	UPROPERTY()
	float CurShoulder;

};
