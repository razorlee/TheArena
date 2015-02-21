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

	/** Current health of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Health;

	/** Current Stamina of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Stamina;

	/** Current Energy of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Energy;

	/** Current Shield of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Shield;

	/** Rate of health regeneration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float HealthRegen;

	/**Rate of stamina regeneration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float StaminaRegen;

	/** Rate of energy regeneration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float EnergyRegen;

	/** Rate of shield regeneration */
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

	/** Increases the amount of damage mitigated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Protection;

	/** Reduces stamina cunsumption and increase stamina regeneration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Mobility;

	/** Decreases movement speed reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Speed;

	/** Increases the players rate of recovery from negative effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Resources)
	float Recovery;

	/** defaults */
	FPlayerData()
	{
		Health = 1000.0f;
		Stamina = 1000.0f;
		Energy = 1000.0f;
		Shield = 0.0f;
		HealthRegen = 5.0f;
		StaminaRegen = 5.0f;
		EnergyRegen = 5.0f;
		ShieldRegen = 0.0f;
		LowHealthPercentage = 0.25f;
		LowStaminaPercentage = 0.0f;
		LowEnergyPercentage = 0.0f;
		LowShieldPercentage = 0.0f;
		Protection = 0.0f;
		Mobility = 0.0f;
		Speed = 0.0f;
		Recovery = 0.0f;
	}
};

UCLASS(Abstract)
class AArenaCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY()
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY()
	class UCameraComponent* FollowCamera;

	class UArenaSaveGame* LoadGameInstance;

	virtual void Tick(float DeltaSeconds) override;

	/** cleanup inventory */
	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	void CameraUpdate();

	UFUNCTION()
	void UpdateCombatState();

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

	/** Afk Timer */
	UPROPERTY(Transient, Replicated)
	float IdleTime;

	/** current targeting state */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CombatState)
	uint8 bInCombat : 1;

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	bool IsEnemyFor(AController* TestPC) const;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	void AddWeapon(class AArenaRangedWeapon* Weapon);

	void RemoveWeapon(class AArenaRangedWeapon* Weapon);

	class AArenaRangedWeapon* FindWeapon(TSubclassOf<class AArenaRangedWeapon> WeaponClass);

	void EquipWeapon(class AArenaRangedWeapon* ToEquip, class AArenaRangedWeapon* ToUnEquip, bool IsEnteringCombat);

	void UnEquipWeapon(class AArenaRangedWeapon* Weapon);

	UFUNCTION()
	void InitializeWeapons(class AArenaRangedWeapon* mainWeapon, class AArenaRangedWeapon* offWeapon);

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
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetCrouched(bool bNewChrouched, bool bToggle);

	/** [server + local] change running state !currently only local! */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetHiCover(bool bNewCover);

	/** [server + local] change running state !currently only local! */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetCover(bool bNewCover);

	/** [server + local] change running state !currently only local! */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetLoCover(bool bNewCover);
	
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetRightEdge(bool bNewEdge);

	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetLeftEdge(bool bNewEdge);

	//////////////////////////////////////////////////////////////////////////
	// Animations

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ThrowAnimation;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* VaultAnimation;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimation;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimation;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimation;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimation;

	/** animation played on pawn (3rd person view) */
	/*UPROPERTY(EditDefaultsOnly, Category = Animation) //tut1
	UAnimMontage* StaggerAnimation;*/

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** [server] performs actual throw */
	virtual void Throw();

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

	/** player pressed throw action */
	void OnThrow();

	/** player pressed start throw action */
	void StartThrow(bool bFromReplication = false);

	/** player released start throw action */
	void StopThrow();

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	/** player pressed cover action */
	void ToggleCover();

	/** player pressed prev weapon action */
	void OnSwapWeapon();

	/** player pressed reload action */
	void OnReload();

	/** player pressed melee action */
	void OnMelee();

	/** player pressed melee action */
	void OnDodge();

	/** player pressed jump action */
	void OnStartJump();

	/** player released jump action */
	void OnStopJump();

	/** player pressed crouch action */
	void OnStartCrouching();

	/** player released crouch action */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void OnStopCrouching();

	/** player pressed run action */
	void OnStartRunning();

	/** player released run action */
	void OnStopRunning();

	/** player press vault action */
	void OnStartVault(bool bFromReplication = false);

	/** player release vault action */
	void OnStopVault();

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

	/** get weapon attach point */
	FName GetOffHandAttachPoint() const;

	/** get weapon attach point */
	FName GetOffWeaponAttachPoint() const;

	/** get weapon attach point */
	FName GetMainWeaponAttachPoint() const;

	/** get weapon attach point */
	FName GetWristOneAttachPoint() const;

	/** get total number of inventory items */
	int32 GetInventoryCount() const;

	class AArenaRangedWeapon* GetInventoryWeapon(int32 index) const;

	/** get the new value for weapon target speed	*/
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	float GetTargetingMovementSpeed() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;

	/** get vaulting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsVaulting() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsCovering() const;

	/** get crouching state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsCrouching() const;

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

	/** get cover state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsHiCovering() const;

	/** get cover state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsLoCovering() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsThrowing() const;

	/** get stagger state */
	/*UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsStaggered() const;*/ //tut1

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
	UFUNCTION(BlueprintCallable, Category = Resources)
	int32 GetMaxEnergy() const;

	/** get current energy */
	UFUNCTION(BlueprintCallable, Category = Resources)
	float GetCurrentEnergy() const;

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

	/** get current IdleTime  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetIdleTime() const;

	/** get current combat state  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool GetCombat() const;

	/** get current direction state  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsLeft() const;

	/** get current direction state  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRight() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRightEdge() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsLeftEdge() const;

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetLeft(bool left, bool right);

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetRight(bool left, bool right);

	/** [server + local] change targeting state !currently only local! */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	void SetCombat(bool bNewCombatState);

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

protected:

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Replicated, Category = Resources)
	FPlayerData PlayerConfig;

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName OffHandAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName OffWeaponAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName MainWeaponAttachPoint;

	/** socket or bone name for attaching Utility mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WristOneAttachPoint;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AArenaRangedWeapon>> DefaultInventoryClasses;

	/** weapons in inventory */
	UPROPERTY(Transient, Replicated)
	TArray<class AArenaRangedWeapon*> Inventory;

	/** play weapon animations */
	float PlayWeaponAnimation(UAnimMontage* Animation);

	/** currently equipped weapon */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AArenaRangedWeapon* CurrentWeapon;

	class AArenaRangedWeapon* LastWeapon;

	class AArenaRangedWeapon* NewWeapon;

	/** main weapon */
	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon)
	class AArenaRangedWeapon* PrimaryWeapon;

	/** secondary weapon */
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	class AArenaRangedWeapon* SecondaryWeapon;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	//////////////////////////////////////////////////////////////////////////
	// Movement

	/** Current movement axis deflecton forward/back (back is negative) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementForwardAxis;

	/** Current movement axis deflecton right/left (left is negative) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float MovementStrafeAxis;

	/** current targeting state */
	UPROPERTY(Transient, Replicated)
	uint8 bIsTargeting : 1;

	/** current running state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToRun : 1;

	/** is weapon fire active? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Vault)
	uint32 bWantsToVault : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToCrouch : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToCover : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToCoverHi : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToCoverLo : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToFaceLeft : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToFaceRight : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bOnLeftEdge : 1;

	/** current crouch state */
	UPROPERTY(Transient, Replicated)
	uint8 bOnRightEdge : 1;

	/** current firing state */
	uint8 bWantsToFire : 1;

	/** current jump state */
	uint8 bWantsToJump : 1;

	/** current throwing state */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Throw)
	uint8 bWantsToThrow : 1;

	/** current throwing state */
	/*UPROPERTY(Transient, ReplicatedUsing = OnRep_Stagger)
	uint8 bPendingStagger : 1;*/ //tut1

	/** flags for dodge direction */
	bool bPressedDodgeRight;
	bool bPressedDodgeLeft;
	bool bPressedDodgeForward;
	bool bPressedDodgeBack;

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
	// Replication

	UFUNCTION()
	void OnRep_Throw();


	UFUNCTION()
	void OnRep_Vault();

	/*UFUNCTION()//tut1
	void OnRep_Stagger();*/

	//////////////////////////////////////////////////////////////////////////
	// Damage & death

public:

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AArenaFragGrenade> GrenadeClass;

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

	/** play stagger on hit */
	//virtual void PlayStagger(); //tut1

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

	/** holsters current weapon */
	void StartEquipWeapon();

	/** equips next weapon */
	void FinishEquipWeapon();

	/** current weapon rep handler */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AArenaRangedWeapon* LastWeapon);

	/** primary weapon rep handler */
	UFUNCTION()
	void OnRep_PrimaryWeapon(class AArenaRangedWeapon* NewWeapon);

	/** secondary weapon rep handler */
	UFUNCTION()
	void OnRep_SecondaryWeapon(class AArenaRangedWeapon* NewWeapon);

	UFUNCTION()
	void OnRep_CombatState(bool bNewCombatState);

	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class AArenaRangedWeapon* ToEquip, class AArenaRangedWeapon* ToUnEquip, bool IsEnteringCombat);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTargeting(bool bNewTargeting);

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartThrow();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopThrow();

	UFUNCTION(reliable, server, WithValidation)
	void ServerInitializeWeapons(AArenaRangedWeapon* mainWeapon, AArenaRangedWeapon* offWeapon);

	/*UFUNCTION(reliable, server, WithValidation) //tut1
	void ServerStartStagger();

	UFUNCTION(reliable, server, WithValidation) //tut1
	void ServerStopStagger();*/

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

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCover(bool bNewCover);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetHiCover(bool bNewCover);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLoCover(bool bNewCover);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRight(bool left, bool right);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLeft(bool left, bool right);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRightEdge(bool bNewEdge);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLeftEdge(bool bNewEdge);

	/** reset idle timer on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerIdleTimer(const float idleTimer, class AArenaCharacter* client);

	/** update combat state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCombat(bool bNewCombatState);
};