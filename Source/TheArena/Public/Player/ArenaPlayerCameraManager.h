#pragma once

#include "Camera/PlayerCameraManager.h"
#include "ArenaPlayerCameraManager.generated.h"

UCLASS()
class THEARENA_API AArenaPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	AArenaPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float TargetingFOV;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	virtual void UpdateCamera(float DeltaTime) override;

};
