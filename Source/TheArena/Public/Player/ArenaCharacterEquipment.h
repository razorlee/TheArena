// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterEquipment.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEARENA_API UArenaCharacterEquipment : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UArenaCharacterEquipment(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts
	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category = Default)
	void Reset();
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION(BlueprintCallable, Category = Weapons)
	AArenaWeapon* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	AArenaWeapon* GetPrimaryWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetPrimaryWeapon(AArenaWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	AArenaWeapon* GetSecondaryWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetSecondaryWeapon(AArenaWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetWeaponAttachPoint();
	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetOffHandAttachPoint();

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetMainWeaponAttachPoint();
	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetOffWeaponAttachPoint();

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetMainHeavyAttachPoint();
	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetOffHeavyAttachPoint();

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetMainPistolAttachPoint();
	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetOffPistolAttachPoint();

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetWristOneAttachPoint();

private:

/////////////////////////////////////////////// Weapons ///////////////////////////////////////////////

	/** currently equipped weapon */
	UPROPERTY(EditAnywhere/*ReplicatedUsing = OnRep_CurrentWeapon*/)
	class AArenaWeapon* CurrentWeapon;
	/** main weapon */

	UPROPERTY(EditAnywhere/*ReplicatedUsing = OnRep_PrimaryWeapon*/)
	class AArenaWeapon* PrimaryWeapon;
	/** secondary weapon */
	UPROPERTY(EditAnywhere, Category = Weapons)
	TSubclassOf<class AArenaWeapon> PrimaryWeaponBP;

	/** secondary weapon */
	UPROPERTY(/*ReplicatedUsing = OnRep_SecondaryWeapon*/)
	class AArenaWeapon* SecondaryWeapon;
	/** secondary weapon */
	UPROPERTY(EditAnywhere, Category = Weapons)
	TSubclassOf<class AArenaWeapon> SecondaryWeaponBP;

/////////////////////////////////////////////// Sockets ///////////////////////////////////////////////

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName WeaponAttachPoint;
	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName OffHandAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName MainWeaponAttachPoint;
	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName OffWeaponAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName MainHeavyAttachPoint;
	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName OffHeavyAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName MainPistolAttachPoint;
	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName OffPistolAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName WristOneAttachPoint;
	
};
