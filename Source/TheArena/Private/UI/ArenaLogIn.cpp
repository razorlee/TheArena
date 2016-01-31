// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaLogIn.h"

void UArenaLogIn::Authenticate(FString userName, FString password)
{
	Http = &FHttpModule::Get();
	
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField("User Name", userName);
	JsonObject->SetStringField("Password", password);

	FString OutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	TSharedRef < IHttpRequest > Request = Http->CreateRequest();
	Request->SetVerb("POST");
	//Request->SetURL(TargetHost + CurrentRequest.TheDest);
	Request->SetHeader("User-Agent", "TheArenaClient/1.0");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(OutputString);
}


