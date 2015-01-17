// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheArena : ModuleRules
{
	public TheArena(TargetInfo Target)
	{
        PrivateIncludePaths.AddRange(
            new string[] { 
				"TheArena/Private/Player",
				"TheArena/Private",
            }
        );

        PublicDependencyModuleNames.AddRange(
           new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
                "Sockets",
                "Networking",
				"AssetRegistry",
                "AIModule",
                "UMG", 
                "Slate", 
                "SlateCore"
			}
       );

       DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");

    }
}
