// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ArenaTypes.h"
#include "ArenaDamageType.h"
#include "GameFramework/Character.h"
#include "ArenaCharacter.generated.h"

USTRUCT()
struct FPlayerData
{
	GENERATED_USTRUCT_BODY()

	// Current health of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Health;

	// Current Stamina of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Stamina;

	// Current Energy of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Energy;

	// Current Shield of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Shield;

	// Rate of health regeneration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float HealthRegen;

	// Rate of stamina regeneration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float StaminaRegen;

	// Rate of energy regeneration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float EnergyRegen;

	// Rate of shield regeneration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float ShieldRegen;

	/** The percentage of health when low health effects should start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float LowHealthPercentage;

	/** The percentage of stamina when low stamina effects should start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float LowStaminaPercentage;

	/** The percentage of energy when low energy effects should start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float LowEnergyPercentage;

	/** The percentage of shield when low shield effects should start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float LowShieldPercentage;

	/** defaults */
	FPlayerData()
	{
		Health = 1000.0f;
		Stamina = 1000.0f;
		Energy = 0.0f;
		Shield = 0.0f;
		HealthRegen = 5.0f;
		StaminaRegen = 5.0f;
		EnergyRegen = 0.0f;
		ShieldRegen = 0.0f;
		LowHealthPercentage = 0.25f;
		LowStaminaPercentage = 0.0f;
		LowEnergyPercentage = 0.0f;
		LowShieldPercentage = 0.0f;
	}
};


UCLASS(Abstract)
class AArenaCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() OVERRIDE;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY()
	TSubobjectPtr<class USpringArmComponent> CameraBoom;

	//UPROPERTY()
	//TSubobjectPtr<class UTimelineComponent> TimeLine;

	/** Follow camera */
	UPROPERTY()
	TSubobjectPtr<class UCameraComponent> FollowCamera;

	virtual void Tick(float DeltaSeconds) OVERRIDE;

	/** cleanup inventory */
	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	void CameraUpdate();

	/** set the default movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	float BaseMovementSpeed;

	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float RunningMovementSpeed;

	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float CrouchedMovementSpeed;

	/** replaces max movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	float SprintCost;

	/** replaces max movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	float JumpCost;

	/** replaces max movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	float DodgeCost;

	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = "Game|Weapon")
	float TargetingMovementSpeed;

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	bool IsEnemyFor(AController* TestPC) const;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	void AddWeapon(class AArenaRangedWeapon* Weapon);

	void RemoveWeapon(class AArenaRangedWeapon* Weapon);

	class AArenaRangedWeapon* FindWeapon(TSubclassOf<class AArenaRangedWeapon> WeaponClass);

	void EquipWeapon(class AArenaRangedWeapon* Weapon);

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] starts weapon fire */
	void StartWeaponFire();

	/** [local] stops weapon fire */
	void StopWeaponFire();

	/** check if pawn can fire weapon */
	bool CanFire() const;

	/** check if pawn can reload weapon */
	bool CanReload() const;

	/** check if pawn can reload weapon */
	bool CanMelee() const;

	/** [server + local] change targeting state !currently only local! */
	void SetTargeting(bool bNewTargeting);

	//////////////////////////////////////////////////////////////////////////
	// Movement

	/** [server + local] change running state !currently only local! */
	void SetRunning(bool bNewRunning, bool bToggle);

	/** [server + local] change running state !currently only local! */
	void SetCrouched(bool bNewChrouched, bool bToggle);

	//////////////////////////////////////////////////////////////////////////
	// Animations

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) OVERRIDE;
	// End of APawn interface

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called via input to turn at a given rate */
	void TurnAtRate(float Rate);

	/** Called via input to turn look up/down at a given rate */
	void LookUpAtRate(float Rate);

	/** player pressed start fire action */
	void OnStartFire();

	/** player released start fire action */
	void OnStopFire();

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	/** player pressed next weapon action */
	void OnNextWeapon();

	/** player pressed prev weapon action */
	void OnPrevWeapon();

	/** player pressed reload action */
	void OnReload();

	/** player pressed reload action */
	//UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnMelee();

	/** player pressed reload action */
	//UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnStopMelee();

	/** player pressed jump action */
	void OnStartJump();

	/** player released jump action */
	void OnStopJump();

	/** player pressed crouch action */
	void OnStartCrouching();

	/** player released crouch action */
	void OnStopCrouching();

	/** player pressed run action */
	void OnStartRunning();

	/** player released run action */
	void OnStopRunning();

	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** get the base movement speed	*/
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetBaseMovementSpeed() const;

	USkeletalMeshComponent* GetPawnMesh() const;

	/** get currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AArenaRangedWeapon* GetWeapon() const;

	/** get weapon attach point */
	FName GetWeaponAttachPoint() const;

	/** get total number of inventory items */
	int32 GetInventoryCount() const;

	class AArenaRangedWeapon* GetInventoryWeapon(int32 index) const;

	/** get the new value for weapon target speed	*/
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	float GetTargetingMovementSpeed() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsFiring() const;

	/** get the new value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetRunningMovementSpeed() const;

	/** get the new value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetCrouchedMovementSpeed() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRunning() const;

	//UFUNCTION(BlueprintCallable, Category = Pawn)
	//bool IsCrouched() const;

	/** get max health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxHealth() const;

	/** get current health */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentHealth() const;

	/** get max stamina */
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxStamina() const;

	/** get current stamina */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentStamina() const;

	/** get max energy */
	//UFUNCTION(BlueprintCallable, Category = Resources)
	//int32 GetMaxEnergy() const;

	/** get current energy */
	//UFUNCTION(BlueprintCallable, Category = Resources)
	//float GetCurrentEnergy() const;

	/** get max shields */
	//UFUNCTION(BlueprintCallable, Category = Resources)
	//int32 GetMaxShields() const;

	/** get current shields */
	//UFUNCTION(BlueprintCallable, Category = Resources)
	//float GetCurrentShields() const;

	/** check if pawn is still alive */
	bool IsAlive() const;

	/** returns percentage of health when low health effects should start */
	float GetLowHealthPercentage() const;

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

protected:

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Resources)
	FPlayerData PlayerConfig;

	/** pawn mesh: 3rd person view */
	//UPROPERTY(VisibleDefaultsOnly, Category = Aesthetics)
	//TSubobjectPtr<USkeletalMeshComponent> Mesh3P;

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AArenaRangedWeapon>> DefaultInventoryClasses;

	/** weapons in inventory */
	UPROPERTY(Transient, Replicated)
	TArray<class AArenaRangedWeapon*> Inventory;

	/** currently equipped weapon */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AArenaRangedWeapon* CurrentWeapon;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** current targeting state */
	UPROPERTY(Transient, Replicated)
	uint8 bIsTargeting : 1;

	/** current running state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToRun : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToCrouch : 1;

	/** current firing state */
	uint8 bWantsToFire : 1;

	/** Should the character be doing damage at the moment? */
	uint8 bDoingMelee;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY()
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY()
	float BaseLookUpRate;

	/** material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* DeathSound;

	/** effect played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	UParticleSystem* RespawnFX;

	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* RespawnSound;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowHealthSound;

	/** sound played when stamina is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowStaminaSound;

	/** sound played when energy is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowEnergySound;

	/** sound played when shield is low */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* LowShieldSound;

	/** create property for running loop sound cue */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundBase* RunLoopSound;

	/** create property for running end sound cue !currently broken! */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* RunStopSound;

	/** sound played when targeting state changes */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundBase* TargetingSound;

	/** manipulates run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

	/** hook to looped low stamina sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowStaminaWarningPlayer;

	/** hook to looped low energy sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowEnergyWarningPlayer;

	/** hook to looped low shield sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowShieldWarningPlayer;

	/** handles sounds for running */
	void UpdateRunSounds(bool bNewRunning);

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();

	/** handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	/** Responsible for cleaning up bodies on clients. */
	virtual void TornOff();

	//////////////////////////////////////////////////////////////////////////
	// Damage & death

public:

	/** collision box for melee */
	UFUNCTION()
	void AttackTrace();

	/** list of actors hit by melee */
	//UPROPERTY(Transient, Replicated)
	TArray<AActor*> HitActors;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Resources)
	uint32 bIsDying : 1;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Resources)
	uint32 bHasShield : 1;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Resources)
	uint32 bHasEnergyGenerator : 1;

	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Pawn suicide */
	virtual void Suicide();

	/** Kill this pawn */
	virtual void KilledBy(class APawn* EventInstigator);

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	// Die when we fall out of the world.
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

protected:

	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** switch to ragdoll */
	void SetRagdollPhysics();

	/** sets up the replication for taking a hit */
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** play hit or death on client */
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** updates current weapon */
	void SetCurrentWeapon(class AArenaRangedWeapon* NewWeapon, class AArenaRangedWeapon* LastWeapon = NULL);

	/** current weapon rep handler */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AArenaRangedWeapon* LastWeapon);

	void SpawnDefaultInventory();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class AArenaRangedWeapon* NewWeapon);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTargeting(bool bNewTargeting);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRunning(bool bNewRunning, bool bToggle);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCrouched(bool bNewCrouched, bool bToggle);

};