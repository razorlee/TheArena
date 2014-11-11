#pragma once

#include "GameFramework/DamageType.h"
#include "ArenaDamageType.generated.h"


UCLASS(const, Blueprintable, BlueprintType)
class THEARENA_API UArenaDamageType : public UDamageType
{
	GENERATED_UCLASS_BODY()

	/** icon displayed in death messages log when killed with this weapon */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon KillIcon;

	/** force feedback effect to play on a player hit by this damage type */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *HitForceFeedback;

	/** force feedback effect to play on a player killed by this damage type */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *KilledForceFeedback;
	
};
