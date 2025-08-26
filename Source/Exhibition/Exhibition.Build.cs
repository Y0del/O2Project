// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Exhibition : ModuleRules
{
    public Exhibition(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HTTP", "Json", "JsonUtilities", "UMG", "Slate", "SlateCore" });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "ApplicationCore",
            "AudioPlatformConfiguration"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd"
            });
        }
    }
}
