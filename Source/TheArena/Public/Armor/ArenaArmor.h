// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaArmor.generated.h"

UENUM()
namespace EArmorType
{
	enum Type
	{
		Head		UMETA(DisplayName = "Head"),
		Shoulders	UMETA(DisplayName = "Shoulders"),
		Chest		UMETA(DisplayName = "Chest"),
		Legs		UMETA(DisplayName = "Legs"),
		Feet		UMETA(DisplayName = "Feet"),
		Hands		UMETA(DisplayName = "Hands")
	};
}

USTRUCT(BlueprintType)
struct FArmorStats 
{
	GENERATED_USTRUCT_BODY()

	/** Increases the amount of damage mitigated */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Protection;

	/** How lightweight the armor is */
	UPROPERTY(EditAnywhere, Category = Stats)
	float Motility;

	///** defaults */
	FArmorStats()
	{
		Protection = 0.0f;
		Motility = 0.0f;
	}
};


UCLASS()
class THEARENA_API AArenaArmor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArenaArmor(const class FObjectInitializer& PCIP);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void Equip();
	void UnEquip();

	UFUNCTION(BlueprintCallable, Category = Defaults)
	class AArenaCharacter* GetMyPawn() const;
	UFUNCTION(BlueprintCallable, Category = Defaults)
	void SetMyPawn(AArenaCharacter* Pawn);

	UFUNCTION(BlueprintCallable, Category = Armor)
	EArmorType::Type GetArmorType();

	UFUNCTION(BlueprintCallable, Category = Config)
	FName GetArmorName() const;

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetProtection() const;
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetProtection(float Value);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetMotility() const;
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetMotility(float Value);

protected:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn, BlueprintReadOnly, Category = Defaults)
	AArenaCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	FName ArmorName;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EArmorType::Type> ArmorType;

	/** stats data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FArmorStats ArmorStats;
	
/////////////////////////////////////// Server ///////////////////////////////////////

	UFUNCTION()
	void OnRep_MyPawn();
};
