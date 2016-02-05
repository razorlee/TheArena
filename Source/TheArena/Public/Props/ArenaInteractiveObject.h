// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "ArenaInteractiveObject.generated.h"

UCLASS()
class THEARENA_API AArenaInteractiveObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArenaInteractiveObject();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Event called when player actually interacts with object
	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnInteract(AArenaCharacter* Player, float InPlayRate = 1.f);
	virtual void OnInteract_Implementation(AArenaCharacter* Player, float InPlayRate = 1.f);

	// Event called when player finishes interacting with object
	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnInteractEnd();
	virtual void OnInteractEnd_Implementation();

	// Event called when player looks at interactable object
	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnView(AArenaCharacter* Player);
	virtual void OnView_Implementation(AArenaCharacter* Player);

	// Event called when player stops looking at interactable object
	UFUNCTION(BlueprintNativeEvent, Category = "Interactable")
	void OnLeave(AArenaCharacter* Player);
	virtual void OnLeave_Implementation(AArenaCharacter* Player);


	////////////// Getters & Setters /////////////////
	UFUNCTION(BlueprintPure, Category = "Interactable")
	bool IsActive();

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void SetActive(bool isActive);

	UFUNCTION(BlueprintPure, Category = "Interactable")
	FText GetInteractText();

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void SetInteractText(FText newText);

	UFUNCTION(BlueprintPure, Category = "Interactable")
	UAnimMontage* GetInteractAnimation();

	UFUNCTION(BlueprintCallable, Category = "Interactable")
	void SetInteractAnimation(UAnimMontage* newAnim);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable", Meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	// Whether Object is currently interactable
	bool bActive;

protected:

	// Interact Text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable", meta = (BlueprintProtected = "true"))
	FText InteractText;

	// Interact Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable", meta = (BlueprintProtected = "true"))
	UAnimMontage* InteractAnimation;

	FTimerHandle InteractTimer;
	AArenaCharacter* InteractedPlayer;
};
