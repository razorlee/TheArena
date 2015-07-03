// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaCharacterMovement.generated.h"

UCLASS()
class THEARENA_API UArenaCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UArenaCharacterMovement(const FObjectInitializer& ObjectInitializer);

public:
	virtual float GetMaxSpeed() const override;

	UFUNCTION(BlueprintCallable, Category = Movement)
	void ManageState(float DeltaSeconds);



	UFUNCTION(BlueprintCallable, Category = Movement)
	float GetTurnRate();

	UFUNCTION(BlueprintCallable, Category = Movement)
	float GetLookUpRate();

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	float GetJumpCost();
	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetJumpCost(float cost);

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	USoundBase* GetRunLoopSound();

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	USoundBase* GetRunStopSound();

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	UAudioComponent* GetRunLoopAC();
	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetRunLoopAC(UAudioComponent* AudioComp);

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetVaultAnimation();

	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Cover)
	FName GetDirection();
	/** set current direction state  */
	UFUNCTION(BlueprintCallable, Category = Cover)
	void SetDirection(FName Direction);

private:

/////////////////////////////////////////// Movement Defaults ///////////////////////////////////////////

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY()
	float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY()
	float BaseLookUpRate;

/////////////////////////////////////////////// Movement Speed ///////////////////////////////////////////////

	/** set the default movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float BaseMovementSpeed;
	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float RunningMovementSpeed;
	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float CrouchedMovementSpeed;
	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float TargetingMovementSpeed;

/////////////////////////////////////////////// Movement Cost ///////////////////////////////////////////////

	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float SprintCost;
	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float JumpCost;
	/** replaces max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float DodgeCost;

////////////////////////////////////////////// Movement Sound //////////////////////////////////////////////

	/** create property for running loop sound cue */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundBase* RunLoopSound;
	/** create property for running end sound cue !currently broken! */
	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* RunStopSound;
	/** manipulates run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

/////////////////////////////////////////// Movement Animation ///////////////////////////////////////////

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* VaultAnimation;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimStart;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimLoop;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimEnd;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimStart;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimLoop;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimEnd;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimStart;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimLoop;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimEnd;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimStart;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimLoop;
	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimEnd;

/////////////////////////////////////////// Movement States ///////////////////////////////////////////

	/** current facing right */
	UPROPERTY()
	FName FaceDirection;

};
