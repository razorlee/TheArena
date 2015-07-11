// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaWeapon.generated.h"

UENUM(BlueprintCallable, BlueprintType, Category = Weapon)
namespace EWeaponClass
{
	enum Type
	{
		AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
		MarksmenRifle	UMETA(DisplayName = "Marksmen Rifle"),
		Shotgun			UMETA(DisplayName = "Shotgun"),
		HeavyWeapon		UMETA(DisplayName = "Heavy Weapon"),
		Pistol			UMETA(DisplayName = "Pistol")
	};
}


UCLASS()
class THEARENA_API AArenaWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArenaWeapon(const class FObjectInitializer& PCIP);

	/** perform initial setup */
	void PostInitializeComponents() override;

	/** destroys */
	void Destroyed() override;

////////////////////////////////////////// Input handlers //////////////////////////////////////////

	/** starts the weapons attack */
	virtual void StartAttack();

	/** stops the weapons attack */
	virtual void StopAttack();

	/** starts the weapons reload */
	virtual void StartReload();

	/** stops the weapons reload */
	virtual void StopReload();

	/** starts the weapons reload */
	virtual void StartMelee();

	/** stops the weapons reload */
	virtual void StopMelee();

	/** weapon is being equipped by owner pawn */
	void Equip();

	/** weapon is now equipped by owner pawn */
	void FinishEquip();

	/** weapon is unequipped by owner pawn */
	float UnEquip();

	/** weapon is now equipped by owner pawn */
	void FinishUnEquip();

////////////////////////////////////////// Sound Controls //////////////////////////////////////////

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

//////////////////////////////////////// Animation Controls ////////////////////////////////////////

	/** play weapon animations */
	float PlayWeaponAnimation(class UAnimMontage* Animation, float InPlayRate = 1.f);

	/** stop playing weapon animations */
	void StopWeaponAnimation(class UAnimMontage* Animation);

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;

///////////////////////////////////////// Socket Controls /////////////////////////////////////////

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

/////////////////////////////////////// Getters and Setters ///////////////////////////////////////

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	class AArenaCharacter* GetPawnOwner() const;
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetOwningPawn(AArenaCharacter* AArenaCharacter);

	UFUNCTION(BlueprintCallable, Category = Config)
	EWeaponClass::Type GetWeaponClass();
	UFUNCTION(BlueprintCallable, Category = Config)
	void SetWeaponClass(EWeaponClass::Type NewClass);

	UFUNCTION(BlueprintCallable, Category = WeaponInfo)
	bool IsPrimary();
	UFUNCTION(BlueprintCallable, Category = WeaponInfo)
	void SetPrimary(bool Status);

	/** get name of the weapon */
	UFUNCTION(BlueprintCallable, Category = WeaponInfo)
	FName GetWeaponName() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponState* GetWeaponState(); //PURE_VIRTUAL(AArenaWeapon::GetWeaponState, );

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual class UArenaRangedWeaponAttributes* GetWeaponAttributes(); //PURE_VIRTUAL(AArenaWeapon::GetWeaponAttributes, );

	UFUNCTION(BlueprintImplementableEvent, Category = Weapon)
	void RotateWeapon();

protected:

	AArenaCharacter* MyPawn;

	UPROPERTY(EditDefaultsOnly, Category = Config)
	TEnumAsByte<EWeaponClass::Type> WeaponClass;

	UPROPERTY(VisibleAnywhere, Category = WeaponInfo)
	bool PrimaryWeapon;

	/** Name of the weapon */
	UPROPERTY(EditDefaultsOnly, Category = WeaponInfo)
	FName WeaponName;

	/** weapon mesh: 3rd person view */
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** holster sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* UnEquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* EquipAnim;

	/** holster animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* UnEquipAnim;

///////////////////////////////////////////////////////////// Server ///////////////////////////////////////////////////////////////

	UFUNCTION(reliable, server, WithValidation)
	void ServerEquip();

	UFUNCTION(reliable, server, WithValidation)
	void ServerUnEquip();

};
