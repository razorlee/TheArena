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

	UFUNCTION(BlueprintCallable, Category = Movement)
	float GetJumpCost();
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetJumpCost(float cost);

	UFUNCTION(BlueprintCallable, Category = Movement)
	USoundBase* GetRunLoopSound();

	UFUNCTION(BlueprintCallable, Category = Movement)
	USoundBase* GetRunStopSound();

	UFUNCTION(BlueprintCallable, Category = Movement)
	UAudioComponent* GetRunLoopAC();
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetRunLoopAC(UAudioComponent* AudioComp);

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetVaultAnimation();

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetClimbAnimation();

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetHighLeftAnimation(FString Sequence);

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetHighRightAnimation(FString Sequence);

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetLowLeftAnimation(FString Sequence);

	UFUNCTION(BlueprintCallable, Category = Cover)
	UAnimMontage* GetLowRightAnimation(FString Sequence);

	UFUNCTION(BlueprintCallable, Category = Cover)
	FName GetDirection();
	UFUNCTION(BlueprintCallable, Category = Cover)
	void SetDirection(FName Direction);

	UFUNCTION(BlueprintCallable, Category = Movement)
	FVector GetCoverDirection();
	UFUNCTION(BlueprintCallable, Category = Movement)
	void SetCoverDirection(FVector NewDirection);

	UFUNCTION(BlueprintCallable, Category = Cover)
	FVector GetLocation();
	UFUNCTION(BlueprintCallable, Category = Cover)
	void SetLocation(FVector NewLocation);

	UFUNCTION(BlueprintCallable, Category = Sockets)
	FName GetLowCover();

private:

/////////////////////////////////////////// Movement Defaults ///////////////////////////////////////////

	UPROPERTY()
	float BaseTurnRate;
	UPROPERTY()
	float BaseLookUpRate;

/////////////////////////////////////////////// Movement Speed ///////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float BaseMovementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float RunningMovementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float CrouchedMovementSpeed;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float TargetingMovementSpeed;

/////////////////////////////////////////////// Movement Cost ///////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float SprintCost;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float JumpCost;

	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float DodgeCost;

////////////////////////////////////////////// Movement Sound //////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundBase* RunLoopSound;

	UPROPERTY(EditDefaultsOnly, Category = Aesthetics)
	USoundCue* RunStopSound;

	UPROPERTY()
	UAudioComponent* RunLoopAC;

/////////////////////////////////////////// Movement Animation ///////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* VaultAnimation;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ClimbAnimation;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimStart;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimLoop;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiLeftAnimEnd;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimStart;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimLoop;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimHiRightAnimEnd;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimStart;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimLoop;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoLeftAnimEnd;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimStart;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimLoop;
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* AimLoRightAnimEnd;

/////////////////////////////////////////// Movement States ///////////////////////////////////////////

	UPROPERTY(Transient, Replicated)
	FName FaceDirection;

	UPROPERTY()
	FVector Direction;

	UPROPERTY()
	FVector Location;

////////////////////////////////////////////// MSockets //////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, Category = Sockets)
	FName LowCover;

/////////////////////////////////////////////// Server ///////////////////////////////////////////////

	//UFUNCTION(reliable, server, WithValidation)
	//void ServerSwapWeapon();

	UFUNCTION(reliable, server, WithValidation)
	void ServerFaceDirection(FName NewFaceDirection);

};
