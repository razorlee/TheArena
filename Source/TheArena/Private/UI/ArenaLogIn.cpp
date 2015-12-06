// Fill out your copyright notice in the Description page of Project Settings.

#include "TheArena.h"
#include "ArenaLogIn.h"

void UArenaLogIn::Authenticate()
{
	std::string password(TCHAR_TO_UTF8(*_password));
	// hash password
}


