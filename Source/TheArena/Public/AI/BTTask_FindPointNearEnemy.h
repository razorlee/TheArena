#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ArenaAIController.h"
#include "BTTask_FindPointNearEnemy.generated.h"

UCLASS()
class THEARENA_API UBTTask_FindPointNearEnemy : public UBTTask_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) override;
};
