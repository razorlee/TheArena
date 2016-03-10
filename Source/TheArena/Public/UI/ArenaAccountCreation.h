// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Http.h"
#include "Blueprint/UserWidget.h"
#include "ArenaAccountCreation.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API UArenaAccountCreation : public UUserWidget
{
	GENERATED_BODY()
	
public:

	FHttpModule* Http;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Region;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString FirstName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString LastName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Email;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Password;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString ConfirmPassword;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString SecurityQuestion;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString SecurityAnswer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	int32 Age;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	bool NewsLetter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	bool Agreement;
	
	
};
