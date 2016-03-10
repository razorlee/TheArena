// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "ArenaLogIn.generated.h"

/**
 * 
 */
UCLASS()
class THEARENA_API UArenaLogIn : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Authenticate")
	void Authenticate(FString userName, FString password);

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FHttpModule* Http;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authenticate")
	FString _userName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authenticate")
	FString _password;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authenticate")
	bool _authenticated;
	
};
