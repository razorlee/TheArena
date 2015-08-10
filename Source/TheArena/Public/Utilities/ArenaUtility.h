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

UCLASS()
class THEARENA_API AArenaUtility : public AActor
{
	GENERATED_BODY()
	
public:	

	AArenaUtility();

	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = Defaults)
	class AArenaCharacter* GetMyPawn() const;
	UFUNCTION(BlueprintCallable, Category = Defaults)
	void SetMyPawn(AArenaCharacter* Pawn);

	UFUNCTION(BlueprintCallable, Category = Config)
	FName GetUtilityName() const;

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

	UFUNCTION(BlueprintCallable, Category = Defaults)
	void ConsumeEnergy( float DeltaSeconds = 1 );
		
protected:

	UPROPERTY(BlueprintReadOnly, Category = Defaults)
	AArenaCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Defaults)
	USkeletalMeshComponent* Mesh3P;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FName UtilityName;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EUtilityType::Type> UtilityType;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EActivationType::Type> ActivationType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Utility)
	bool Active;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Config)
	float ActivationCost;
	
};
