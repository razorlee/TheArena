// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaUtility.generated.h"

UENUM()
namespace EUtilityType
{
	enum Type
	{
		Head		UMETA(DisplayName = "Head"),
		UpperBack	UMETA(DisplayName = "Upper Back"),
		LowerBack	UMETA(DisplayName = "Lower Back"),
		Wrist		UMETA(DisplayName = "Wrist"),
		Waist		UMETA(DisplayName = "Waist")
	};
}

UENUM()
namespace EActivationType
{
	enum Type
	{
		Activate	UMETA(DisplayName = "Activated"),
		Toggle		UMETA(DisplayName = "Toggled"),
		Channel		UMETA(DisplayName = "Channeled")
	};
}


USTRUCT()
struct FUtilityStats
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EUtilityType::Type> UtilityType;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EActivationType::Type> ActivationType;

	/*The cost to initially activate the ability*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Config)
	float ActivationCost;

	/*The cost to sustain channeled or toggled ability per second*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Config)
	float ContinuationCost;

	/** The damage dealt to player before defense mitigation */
	UPROPERTY(EditDefaultsOnly, Category = Stats/*, meta = (ClampMin = "1", ClampMax = "1000", UIMin = "1", UIMax = "1000")*/)
	int32 Damage;

	/** The velocity of the projectile in Unreal Units */
	UPROPERTY(EditDefaultsOnly, Category = Stats/*, meta = (ClampMin = "5000", ClampMax = "45000", UIMin = "5000", UIMax = "45000")*/)
	float Velocity;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	bool IsExplosive;

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (EditCondition = "IsExplosive"))
	int32 ExplosionRadius;

	/** defaults */
	FUtilityStats()
	{
		Damage = 100;
		Velocity = 37600.0f;
		ActivationCost = 0.0f;
		ContinuationCost = 0.0f;
		IsExplosive = true;
		ExplosionRadius = 1000.0f;
	}
};

UCLASS()
class THEARENA_API AArenaUtility : public AActor
{
	GENERATED_BODY()
	
public:	

	AArenaUtility(const class FObjectInitializer& PCIP);

	void Destroyed() override;

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = Defaults)
	class AArenaCharacter* GetMyPawn() const;
	UFUNCTION(BlueprintCallable, Category = Defaults)
	void SetMyPawn(AArenaCharacter* Pawn);

	UFUNCTION(BlueprintCallable, Category = Config)
	FName GetUtilityName() const;

	UFUNCTION(BlueprintCallable, Category = Stats)
	FString GetDescription();
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetDescription(FString Value);

	UFUNCTION(BlueprintCallable, Category = Config)
	float GetActivationCost() const;
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetActivationCost(float Value);

	void Equip();
	void UnEquip();

	UFUNCTION(BlueprintCallable, Category = Utility)
	EUtilityType::Type GetUtilityType();

	UFUNCTION(BlueprintCallable, Category = Utility)
	EActivationType::Type GetActivationType();

	virtual void Activate();
	UFUNCTION(BlueprintImplementableEvent, Category = Utility)
	void ActivateBP();

	virtual void Deactivate();
	UFUNCTION(BlueprintImplementableEvent, Category = Utility)
	void DeactivateBP();

	UFUNCTION(BlueprintCallable, Category = Utility)
	void FireProjectile();
	FHitResult GetAdjustedAim();
	FVector GetSocketLocation();

	UFUNCTION(BlueprintCallable, Category = Defaults)
	void ConsumeEnergy(float Cost, float DeltaSeconds = 1);

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Utility)
	bool Active;

	UFUNCTION(NetMultiCast, Reliable)
	void PlayUtilityAnimation();
		
protected:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn, BlueprintReadOnly, Category = Defaults)
	AArenaCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FName UtilityName;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FString UtilityDescription;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FUtilityStats UtilityStats;

	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AArenaProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadWrite, Category = Config)
	TEnumAsByte< ECollisionChannel > Channel;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* UtilityAnim;

/////////////////////////////////////// Server ///////////////////////////////////////

	UFUNCTION()
	void OnRep_MyPawn();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerDeactivate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerActivate();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSpawnProjectile(FVector Origin, FVector ShootDir, FHitResult Hit);

	void Test(FVector Origin, FVector ShootDir, FHitResult Hit);

};
