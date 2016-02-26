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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authenticate")
	FString _userName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authenticate")
	FString _password;
	
	
};
