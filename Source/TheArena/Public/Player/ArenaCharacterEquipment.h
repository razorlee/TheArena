// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ArenaCharacterEquipment.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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

	UFUNCTION(BlueprintCallable, Category = Character)
	class AArenaCharacter* GetMyPawn() const;
	UFUNCTION(BlueprintCallable, Category = Character)
	void SetMyPawn(AArenaCharacter* Pawn);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	FString GetCurrentWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetCurrentWeapon(FString Value);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaWeapon> GetPrimaryWeaponBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetPrimaryWeaponBP(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaWeapon> GetSecondaryWeaponBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetSecondaryWeaponBP(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetHeadUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetHeadUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetUpperBackUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetUpperBackUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetLowerBackUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetLowerBackUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetLeftWristUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetLeftWristUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetRightWristUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetRightWristUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetLeftWaistUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetLeftWaistUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	TSubclassOf<class AArenaUtility> GetRightWaistUtilityBP();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetRightWaistUtilityBP(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	bool GetDrawCrosshair();
	UFUNCTION(BlueprintCallable, Category = Weapons)
	void SetDrawCrosshair(bool Allow);

////////////////////////////////////////// Armor Getter and Setter /////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Armor)
	TSubclassOf<class AArenaArmor> GetHeadArmorBP();
	UFUNCTION(BlueprintCallable, Category = Armor)
	void SetHeadArmorBP(TSubclassOf<class AArenaArmor> Armor);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	UPROPERTY()
	AArenaCharacter* MyPawn;

	UPROPERTY(EditAnywhere, Replicated, Category = Weapons)
	FString CurrentWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_PrimaryWeapon)
	class AArenaWeapon* PrimaryWeapon;
	UPROPERTY(EditAnywhere, Replicated, Category = Weapons)
	TSubclassOf<class AArenaWeapon> PrimaryWeaponBP;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_SecondaryWeapon)
	class AArenaWeapon* SecondaryWeapon;
	UPROPERTY(EditAnywhere, Replicated, Category = Weapons)
	TSubclassOf<class AArenaWeapon> SecondaryWeaponBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> HeadUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> UpperBackUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> LowerBackUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> LeftWristUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> RightWristUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> LeftWaistUtilityBP;

	UPROPERTY(EditAnywhere, Replicated, Category = Utilities)
	TSubclassOf<class AArenaUtility> RightWaistUtilityBP;

///////////////////////////////////////////////// Armor ////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Replicated, Category = Armor)
	TSubclassOf<class AArenaArmor> HeadArmorBP;

/////////////////////////////////////////////// Booleans ///////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = Weapons)
	bool DrawCrosshair;

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

/////////////////////////////////////////////// Server ///////////////////////////////////////////////

	UFUNCTION()
	void OnRep_PrimaryWeapon(class AArenaWeapon* Weapon);

	UFUNCTION()
	void OnRep_SecondaryWeapon(class AArenaWeapon* Weapon);
};
