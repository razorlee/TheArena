#pragma once

#include "ArenaAIController.h"
#include "ArenaAI.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTTask_HasLosTo.generated.h"

UCLASS()
class THEARENA_API UBTTask_HasLosTo : public UBTDecorator
{
	GENERATED_UCLASS_BODY()

	//virtual EBTNodeResult::Type ExecuteTask(class UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(class UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) const override;

protected:

	UPROPERTY(EditAnywhere, Category = Condition)
	struct FBlackboardKeySelector EnemyKey;

private:
	bool LOSTrace(AActor* InActor, AActor* InEnemyActor, const FVector& EndLocation) const;
};
