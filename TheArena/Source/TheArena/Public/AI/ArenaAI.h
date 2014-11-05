#pragma once

#include "ArenaCharacter.h"
#include "ArenaAI.generated.h"

UCLASS()
class THEARENA_API AArenaAI : public AArenaCharacter
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* BotBehavior;

	virtual void FaceRotation(FRotator NewRotation, float DeltaTime = 0.f) override;
};
