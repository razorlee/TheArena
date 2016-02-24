// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ArenaTypes.h"
#include "ArenaCharacterState.h"
#include "ArenaDamageType.h"
#include "GameFramework/Character.h"
#include "Http.h"
#include "ArenaCharacter.generated.h"

UCLASS(Abstract)
class AArenaCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

public:

	//AArenaCharacter::AArenaCharacter(const class FObjectInitializer& PCIP);

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

	/** spawn inventory, setup initial variables */
	virtual void BeginPlay() override;

	void SaveCharacter();

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

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
	/** player pressed vault action */
	void OnVault();
	/** player pressed climb action */
	void OnClimb();
	/** player pressed crouch action */
	void OnCrouch();
	/** player pressed cover action */
	void OnToggleCover();
	/** player pressed exit cover action */
	UFUNCTION(BlueprintCallable, Category = Input)
		void OnExitCover();
	/** player pressed run action */
	void OnStartRunning();
	/** player released run action */
	void OnStopRunning();
	/** player pressed targeting action */
	void OnStartTargeting();
	/** player released targeting action */
	void OnStopTargeting();
	/** player pressed peak action */
	void OnStartPeaking();
	/** player released peak action */
	void OnStopPeaking();
	/** player enters combat */
	void OnToggleCombat();
	/** player pressed prev weapon action */
	void OnSwapWeapon();
	/** player pressed prev weapon action */
	void OnSwitchShoulder();
	/** player pressed reload action */
	void OnReload();
	/** player pressed melee action */
	void OnMelee();
	/** player pressed dodge action */
	void OnDodge();
	/** player pressed start fire action */
	void OnStartFire();
	/** player released stop fire action */
	void OnStopFire();
	/** player pressed start back action */
	void OnActivateBack();
	/** player released stop back action */
	void OnDeactivateBack();
	/** player pressed start waist action */
	void OnActivateLeftWaist();
	/** player released stop waist action */
	void OnDeactivateLeftWaist();
	/** player pressed start waist action */
	void OnActivateRightWaist();
	/** player released stop waist action */
	void OnDeactivateRightWaist();
	/** player pressed start wrist action */
	void OnActivateLeftWrist();
	/** player released stop wrist action */
	void OnDeactivateLeftWrist();
	/** player pressed start wrist action */
	void OnActivateRightWrist();
	/** player released stop wrist action */
	void OnDeactivateRightWrist();
	/** player activated a targeting utility */
	void OnUtilityStartTarget(AArenaUtility* Utility);
	/** player stopped a targeting utility */
	void OnUtilityStopTarget();
	/** player canceled an action */
	void OnCancelAction();

	////////////////////////////////////////// Character Defaults //////////////////////////////////////////

	virtual void Tick(float DeltaSeconds) override;

	/** cleanup inventory */
	virtual void Destroyed() override;

	/** update mesh for first person view */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		FRotator GetAimOffsets() const;

	UFUNCTION()
		bool GetBusy();
	UFUNCTION(NetMultiCast, Unreliable)
		void ToggleBusy();

	UFUNCTION(NetMultiCast, Unreliable)
		void SetLocation();

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

	UFUNCTION(BlueprintCallable, Category = Mesh)
		TArray<UMaterialInstanceDynamic*> GetMeshMIDs();

	UFUNCTION(BlueprintCallable, Category = Character)
		FString GetName() const;
	UFUNCTION(NetMulticast, Unreliable)
		void SetName(const FString& NewName);

	UFUNCTION()
	bool GetTargeting();

	UFUNCTION()
	bool GetUtilityTargeting();

	/////////////////////////////////////// Get and Set Weapons ///////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class AArenaWeapon* GetCurrentWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
		void SetCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class AArenaWeapon* GetPrimaryWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
		void SetPrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);
	UFUNCTION(BlueprintCallable, Category = Weapons)
		void HandlePrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(BlueprintCallable, Category = Weapons)
	class AArenaWeapon* GetSecondaryWeapon();
	UFUNCTION(BlueprintCallable, Category = Weapons)
		void SetSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);
	UFUNCTION(BlueprintCallable, Category = Weapons)
		void HandleSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	/////////////////////////////////////// Get and Set Utilities ///////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetHeadUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetHeadUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleHeadUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetUpperBackUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetUpperBackUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleUpperBackUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetLowerBackUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetLowerBackUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleLowerBackUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetLeftWristUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetLeftWristUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleLeftWristUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetRightWristUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetRightWristUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleRightWristUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetLeftWaistUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(BlueprintCallable, Category = Utilities)
	class AArenaUtility* GetRightWaistUtility();
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void SetRightWaistUtility(TSubclassOf<class AArenaUtility> Utility);
	UFUNCTION(BlueprintCallable, Category = Utilities)
		void HandleRightWaistUtility(TSubclassOf<class AArenaUtility> Utility);

	/////////////////////////////////////// Get and Set Utilities ///////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetHeadArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetHeadArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleHeadArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetChestArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetChestArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleChestArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetShoulderArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetShoulderArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleShoulderArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetHandArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetHandArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleHandArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetLegArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetLegArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleLegArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(BlueprintCallable, Category = Armor)
	class AArenaArmor* GetFeetArmor();
	UFUNCTION(BlueprintCallable, Category = Armor)
		void SetFeetArmor(TSubclassOf<class AArenaArmor> Armor);
	UFUNCTION(BlueprintCallable, Category = Armor)
		void HandleFeetArmor(TSubclassOf<class AArenaArmor> Armor);

	////////////////////////////////////////// Animation Controls //////////////////////////////////////////

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	////////////////////////////////////////// Action Functions //////////////////////////////////////////

	void SpawnEquipment();
	void LoadPersistence();
	void AddWeapon(class AArenaWeapon* Weapon);
	void RemoveWeapon(class AArenaWeapon* Weapon);

	void InitializeWeapons();

	UFUNCTION(BlueprintCallable, Category = Weapons)
		void ApplyArmorStats();

	UFUNCTION(NetMultiCast, Reliable)
		void ToggleCrouch();

	UFUNCTION(NetMultiCast, Reliable)
		void ToggleCover();

	UFUNCTION(NetMultiCast, Reliable)
		void ExitCover();

	UFUNCTION(NetMultiCast, Reliable)
		void Running(bool IsRunning);

	UFUNCTION(NetMultiCast, Reliable)
		void StartTargeting();
	UFUNCTION(NetMultiCast, Reliable)
		void StopTargeting();

	UFUNCTION(NetMultiCast, Reliable)
		void StartPeaking();
	UFUNCTION(NetMultiCast, Reliable)
		void StopPeaking();

	UFUNCTION(NetMultiCast, Reliable)
		void ToggleCombat();

	UFUNCTION(NetMultiCast, Reliable)
		void SwapWeapon();

	void EquipWeapon();
	void FinishEquipWeapon(class AArenaWeapon* Weapon);

	float UnEquipWeapon();
	float FinishUnEquipWeapon(class AArenaWeapon* Weapon);

	UFUNCTION(NetMultiCast, Reliable)
		void StartVault();
	UFUNCTION(NetMultiCast, Reliable)
		void StopVault();

	UFUNCTION(NetMultiCast, Reliable)
		void StartClimb();
	UFUNCTION(NetMultiCast, Reliable)
		void StopClimb();

	UFUNCTION(NetMultiCast, Reliable)
	void CancelAction();

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

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();
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

	UPROPERTY(BlueprintReadOnly, Transient, Replicated)
		uint32 Peaking : 1;

protected:

	UPROPERTY()
		bool Spawned;

	UPROPERTY()
		bool ReadySpawned;

	UPROPERTY(Replicated)
		FString Name;

	FHttpModule* Http;

	FString TargetHost;

	////////////////////////////////////////// Private Properties //////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = State)
	class UArenaCharacterState* CharacterState;

	UPROPERTY(EditAnywhere, Replicated, Category = Attributes)
	class UArenaCharacterAttributes* CharacterAttributes;

	UPROPERTY(EditAnywhere, Replicated, Category = Equipment)
	class UArenaCharacterEquipment* CharacterEquipment;

	UPROPERTY(EditAnywhere, Category = Inventory)
	class UArenaCharacterInventory* CharacterInventory;

	class UArenaCharacterMovement* CharacterMovementComponent;

	class UArenaSaveGame* SaveGameInstance;

	////////////////////////////////////////// Weapons //////////////////////////////////////////

	UPROPERTY(Transient, Replicated/*ReplicatedUsing = OnRep_CurrentWeapon*/)
	class AArenaWeapon* CurrentWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_PrimaryWeapon)
	class AArenaWeapon* PrimaryWeapon;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_SecondaryWeapon)
	class AArenaWeapon* SecondaryWeapon;

	////////////////////////////////////////// Utilities //////////////////////////////////////////

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* HeadUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* UpperBackUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* LowerBackUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* LeftWristUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* RightWristUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* LeftWaistUtility;

	UPROPERTY(Transient, Replicated)
	class AArenaUtility* RightWaistUtility;

	////////////////////////////////////////// Armor //////////////////////////////////////////

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* ChestArmor;

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* HandArmor;

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* HeadArmor;

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* FeetArmor;

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* LegArmor;

	UPROPERTY(Transient, Replicated)
	class AArenaArmor* ShoulderArmor;

	////////////////////////////////////////// Private Properties //////////////////////////////////////////

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_SwapWeapon;

	FTimerHandle TimerHandle_Busy;

	FTimerHandle TimerHandle_SetLocation;

	FTimerHandle TimerHandle_StopPeaking;

	FTimerHandle TimerHandle_Vault;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(Transient, Replicated)
	uint32 Busy : 1;

	/** Identifies if pawn is in its dying state */
	UPROPERTY(Transient, Replicated)
	float ActionQueue;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(BlueprintReadWrite, Transient, Category = Mesh)
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

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** [client] perform PlayerState related setup */
	virtual void OnRep_PlayerState() override;

	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	UFUNCTION()
	void OnRep_PrimaryWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	////////////////////////////////////////////// Server //////////////////////////////////////////////

	UFUNCTION(reliable, server, WithValidation)
	void ServerApplyArmorStats();

	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon();

	UFUNCTION(reliable, server, WithValidation)
	void ServerUnEquipWeapon();

	UFUNCTION(reliable, server, WithValidation)
	void ServerJump(class AArenaCharacter* client);

	UFUNCTION(reliable, server, WithValidation)
	void ServerVault();

	UFUNCTION(reliable, server, WithValidation)
	void ServerClimb();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartTargeting();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopTargeting();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartUtilityTarget();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopUtilityTarget();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartPeaking();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopPeaking();

	UFUNCTION(reliable, server, WithValidation)
	void ServerCancelAction();

	/////////////////////////////////////////////////////////////////////////

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetName(const FString& NewName);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSpawnEquipment(
		TSubclassOf<class AArenaWeapon> MainWeapon,
		TSubclassOf<class AArenaWeapon> OffWeapon,
		TSubclassOf<class AArenaUtility> Head,
		TSubclassOf<class AArenaUtility> UpperBack,
		TSubclassOf<class AArenaUtility> LowerBack,
		TSubclassOf<class AArenaUtility> LeftWaist,
		TSubclassOf<class AArenaUtility> RightWaist,
		TSubclassOf<class AArenaUtility> LeftWrist,
		TSubclassOf<class AArenaUtility> RightWrist,
		TSubclassOf<class AArenaArmor> HeadA,
		TSubclassOf<class AArenaArmor> ShoulderA,
		TSubclassOf<class AArenaArmor> ChestA,
		TSubclassOf<class AArenaArmor> HandsA,
		TSubclassOf<class AArenaArmor> LegsA,
		TSubclassOf<class AArenaArmor> FeetA);

	UFUNCTION(reliable, server, WithValidation)
	void ServerToggleCrouch();

	UFUNCTION(reliable, server, WithValidation)
	void ServerToggleCover();

	UFUNCTION(reliable, server, WithValidation)
	void ServerExitCover();

	UFUNCTION(reliable, server, WithValidation)
	void ServerRunning(bool IsRunning);

	UFUNCTION(reliable, server, WithValidation)
	void ServerEnterCombat();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSwapWeapon();

	UFUNCTION(reliable, server, WithValidation)
	void ServerInitializeWeapons();

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetPrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetHeadUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetUpperBackUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLowerBackUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLeftWristUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRightWristUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRightWaistUtility(TSubclassOf<class AArenaUtility> Utility);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetHeadArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetChestArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetShoulderArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetHandArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetLegArmor(TSubclassOf<class AArenaArmor> Armor);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetFeetArmor(TSubclassOf<class AArenaArmor> Armor);

};
