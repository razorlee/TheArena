#pragma once

#include "AIController.h"
#include "ArenaAI.h"
#include "ArenaCharacter.h"
#include "ArenaAIController.generated.h"

UCLASS()
class THEARENA_API AArenaAIController : public AAIController
{
	GENERATED_UCLASS_BODY()	

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComp;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorComp;

	// Begin AController interface
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	virtual void Possess(class APawn* InPawn) override;
	virtual void BeginInactiveState() override;
	// End APlayerController interface

	void Respawn();

	void CheckAmmo(const class AArenaRangedWeapon* CurrentWeapon);

	void SetEnemy(class APawn* InPawn);

	class AArenaCharacter* GetEnemy() const;

	/* If there is line of sight to current enemy, start firing at them */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	void ShootEnemy();

	/* Finds the closest enemy and sets them as current target */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FindClosestEnemy();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS(AArenaCharacter* ExcludeEnemy);

	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;

	// Begin AAIController interface
	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;
	// End AAIController interface

protected:
	// Check of we have LOS to a character
	bool LOSTrace(AArenaCharacter* InEnemyChar) const;

	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;
};
