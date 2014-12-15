// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindPointNearEnemy::UBTTask_FindPointNearEnemy(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
}

EBTNodeResult::Type UBTTask_FindPointNearEnemy::ExecuteTask(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory)
{
	UBehaviorTreeComponent* MyComp = OwnerComp;
    AArenaAIController* MyController = MyComp ? Cast<AArenaAIController>(MyComp->GetOwner()) : NULL;
	if (MyController == NULL)
	{
		return EBTNodeResult::Failed;
	}

	APawn* MyBot = MyController->GetPawn();
	AArenaCharacter* Enemy = MyController->GetEnemy();
	if (Enemy && MyBot)
	{
		const float SearchRadius = 200.0f;
		const FVector SearchOrigin = Enemy->GetActorLocation() + 200.0f * (MyBot->GetActorLocation() - Enemy->GetActorLocation()).SafeNormal();
		const FVector Loc = UNavigationSystem::GetRandomPointInRadius(MyController, SearchOrigin, SearchRadius);
		if (Loc != FVector::ZeroVector)
		{
			MyComp->GetBlackboardComponent()->SetValueAsVector(BlackboardKey.GetSelectedKeyID(), Loc);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
