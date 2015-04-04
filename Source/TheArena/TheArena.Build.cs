// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheArena : ModuleRules
{
    public TheArena(TargetInfo Target)
    {
        PrivateIncludePaths.AddRange(
            new string[] { 
		        "TheArena/Private/Player",
                "TheArena/Private/Online",
		        "TheArena/Private"
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[] {
		        "Core",
		        "CoreUObject",
		        "Engine",
                "InputCore",
		        "OnlineSubsystem",
                "OnlineSubsystemNull",
		        "OnlineSubsystemUtils",
                "Sockets",
                "Networking",
		        "AssetRegistry",
                "MoviePlayer",
                "AIModule",
                "UMG", 
                "Slate", 
                "SlateCore"
	        }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] { 
                "OnlineSubsystem" 
            }
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

    }
}
