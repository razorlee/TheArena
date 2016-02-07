// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaLogIn.h"

void UArenaLogIn::Authenticate(FString userName, FString password)
{
	Http = &FHttpModule::Get();
	
	/*TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("User Name", userName);
	JsonObject->SetStringField("Password", password);*/

	//FString OutputString;
	//TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	//FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	TSharedRef < IHttpRequest > Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL("192.168.0.43:5000/login");
	Request->SetHeader("User-Agent", "TheArenaClient/1.0");
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->SetContentAsString("username=" + userName + "&password=" + password);
	Request->OnProcessRequestComplete().BindUObject(this, &UArenaLogIn::OnResponseReceived);
	Request->ProcessRequest();
}

void UArenaLogIn::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (!Response.IsValid())
		{
		}
	}
	else
	{

	}
}

