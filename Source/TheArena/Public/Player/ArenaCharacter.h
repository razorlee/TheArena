// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ArenaTypes.h"
#include "ArenaDamageType.h"
#include "GameFramework/Character.h"
#include "ArenaCharacter.generated.h"

UCLASS(Abstract)
class AArenaCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

public:

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

////////////////////////////////////////// Input handlers //////////////////////////////////////////

	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);
	/** Called for side to side input */
	void MoveRight(float Value);
	/** Called via input to turn at a given rate */
	void TurnAtRate(float Rate);
	/** Called via input to turn look up/down at a given rate */
	void LookUpAtRate(float Rate);
	/** player pressed jump action */
	void OnStartJump();
	/** player released jump action */
	void OnStopJump();
	/** player pressed crouch action */
	void OnCrouch();
	/** player pressed cover action */
	void OnCover();
	/** player pressed run action */
	void OnStartRunning();
	/** player released run action */
	void OnStopRunning();
	/** player pressed targeting action */
	void OnStartTargeting();
	/** player released targeting action */
	void OnStopTargeting();
	/** player enters combat */
	void OnEnterCombat();
	/** player pressed prev weapon action */
	void OnSwapWeapon();
	/** player pressed reload action */
	void OnReload();
	/** player pressed melee action */
	void OnMelee();
	/** player pressed melee action */
	void OnDodge();
	/** player pressed start fire action */
	void OnStartFire();
	/** player released start fire action */
	void OnStopFire();

////////////////////////////////////////// Character Defaults //////////////////////////////////////////

	virtual void Tick(float DeltaSeconds) override;

	/** cleanup inventory */
	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

////////////////////////////////////////// Character Components //////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Character)
	USkeletalMeshComponent* GetPawnMesh() const;

	UFUNCTION(BlueprintCallable, Category = Character)
	class UArenaCharacterMovement* GetPlayerMovement();

	UFUNCTION(BlueprintCallable, Category = Character)
	class UArenaCharacterState* GetPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = Character)
	class UArenaCharacterAttributes* GetCharacterAttributes();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class UArenaCharacterEquipment* GetCharacterEquipment();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class UArenaCharacterInventory* GetCharacterInventory();

////////////////////////////////////////// Animation Controls //////////////////////////////////////////

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

////////////////////////////////////////// Action Functions //////////////////////////////////////////

	void SetTargeting(bool bNewTargeting);
	void StartTargeting(bool bFromReplication = false);
	void LoopTargeting();

	void EnterCombat();

	void SwapWeapon();

	void EquipWeapon();
	void FinishEquipWeapon(class AArenaWeapon* Weapon);

	float UnEquipWeapon();
	float FinishUnEquipWeapon(class AArenaWeapon* Weapon);

	void OnStartVault(bool bFromReplication = false);
	void OnStopVault();

	void StartWeaponFire();
	void StopWeaponFire();

////////////////////////////////////////// Damage & Death //////////////////////////////////////////

	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** Kills pawn.  Server/authority only */
	virtual bool Die(float KillingDamage, struct FDamageEvent const& KillingDamageEvent, class AController* Killer, class AActor* DamageCauser);

	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	// Die when we fall out of the world.
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	/** Pawn suicide */
	virtual void Suicide();

	/** Kill this pawn */
	virtual void KilledBy(class APawn* EventInstigator);

////////////////////////////////////////// Pawn Handeling //////////////////////////////////////////

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();
	/** Responsible for cleaning up bodies on clients. */
	virtual void TornOff();

////////////////////////////////////////// Audio Controls //////////////////////////////////////////

	/** handles sounds for running */
	void UpdateRunSounds(bool bNewRunning);

	/** switch to ragdoll */
	void SetRagdollPhysics();

////////////////////////////////////////// Public Properties //////////////////////////////////////////

	/** Camera boom positioning the camera behind the character */
	UPROPERTY()
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY()
	class UCameraComponent* FollowCamera;

protected:

////////////////////////////////////////// Private Properties //////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = State)
	class UArenaCharacterState* CharacterState;

	UPROPERTY(EditAnywhere, Category = Attributes)
	class UArenaCharacterAttributes* CharacterAttributes;

	UPROPERTY(EditAnywhere, Category = Equipment)
	class UArenaCharacterEquipment* CharacterEquipment;

	UPROPERTY(EditAnywhere, Category = Inventory)
	class UArenaCharacterInventory* CharacterInventory;

	class UArenaCharacterMovement* CharacterMovementComponent;

	class UArenaSaveGame* LoadGameInstance;

	//class UServer_ArenaCharacter* Server;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_SwapWeapon;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Resources)
	uint32 bIsDying : 1;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** effect played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	UParticleSystem* RespawnFX;

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* DeathSound;
	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* RespawnSound;

	/** sound played when targeting state changes */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundBase* TargetingSound;

//////////////////////////////////////////// Replication ////////////////////////////////////////////

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	/** sets up the replication for taking a hit */
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	UFUNCTION()
	void OnRep_Vault();
	UFUNCTION()
	void OnRep_Aim();

////////////////////////////////////////////// Server //////////////////////////////////////////////

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerUnEquipWeapon();

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTargeting(bool bNewTargeting);

	UFUNCTION(reliable, server, WithValidation)
	void ServerJump(class AArenaCharacter* client);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRunning(bool bNewRunning, bool bToggle);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCrouched(bool bNewCrouched, bool bToggle);

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartVault();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopVault();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartTargeting();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopTargeting();

	UFUNCTION(reliable, server, WithValidation)
	void ServerEnterCombat();

};